//
// Copyright (c) 2020 Xinyan. All rights reserved.
// Created on 2020/3/31.
//
#include <data_loader.h>
#include <kde_exact.h>
#include <kde_lsh.h>
#include <kde_nsw.h>
#include <kde_qie.h>
#include <kde_rs.h>
#include <kde_vq.h>
#include <stdio.h>

#include <iostream>

using std::cout;
using std::endl;

void test(const T *xs, const T *qs, const int nx, const int nq, const int d) {
  cout << "shape of " << nx << "x" << d << endl;
  cout << "constructing ExactKDE" << endl;
  ExactKDE kde(xs, nx, d);
  cout << "constructing VQE" << endl;
  VQE<2> vqe(xs, nx, d);
  cout << "constructing RandomSample" << endl;
  RandomSample rs(xs, nx, d);
  cout << "constructing HBE" << endl;
  HBE lsh(xs, nx, d, 1, 16, 16, 8.0);

  for (int i = 0; i < nq; ++i) {
    const T *q = qs + i * d;

    printf("\n\nKDE", kde.query(q));
    printf("\t%.5f\n", kde.query(q));
    printf("\nVQE");
    for (auto dist : {0, 2, 4, 8, 16}) {
      printf("\t%.5f\n", vqe.query(q, dist));
    }
    printf("\nHBE");
    for (auto m : {1, 2, 4, 8, 16}) {
      printf("\t%.5f\n", lsh.query(q, 1, m));
    }
    printf("\nRS");
    for (auto m : {2, 4, 8, 16, 20}) {
      printf("\t%.5f\n", rs.query(q, 1, 1u << m));
    }
  }
}

void benchmark_exact(const T *xs, const T *qs, int nx, int nq, int d,
                     vector<T> &gt_density) {
  cout << "constructing ExactKDE" << endl;
  gt_density.resize(nq);
  timer t;
  ExactKDE kde(xs, nx, d);
  printf("construct ExactKDE time \t%.5f\n", t.restart());
  for (int i = 0; i < nq; ++i) {
    gt_density[i] = kde.query(qs + i * d);
  }
  printf("query ExactKDE time \t%.5f\n", t.restart() / nq);
}

void benchmark_graph(const T *xs, const T *qs, int nx, int nq, int d,
                     vector<T> &gt_density) {
  cout << "constructing GraphKDE" << endl;
  vector<T> g_density(nq);
  timer t;
  GraphKDE graph(xs, nx, d);
  printf("construct HNSW time \t%.5f\n", t.restart());
  for (int i = 0; i < nq; ++i) {
    g_density[i] = graph.query(qs + i * d, 32);
  }
  printf("query HNSW time \t%.5f\n", t.restart() / nq);
  printf("query variance \t%.5f\n",
         l2dist_sqr(gt_density.data(), g_density.data(), nq));
}

template <int M>
void benchmark_vq(const T *xs, const T *qs, int nx, int nq, int d,
                  vector<T> &gt_density) {
  cout << "constructing VQE<" << M << ">" << endl;
  vector<T> vq_density(nq);
  timer t;
  VQE<M> vqe(xs, nx, d);
  printf("construct VQE time \t%.5f\n", t.restart());
  for (int i = 0; i < nq; ++i) {
    vq_density[i] = vqe.query(qs + i * d, 0.01);
  }
  printf("query VQE time \t%.5f\n", t.restart() / nq);
  printf("query variance \t%.5f\n",
         l2dist_sqr(gt_density.data(), vq_density.data(), nq));
}

void benchmark_qi(const T *xs, const T *qs, int nx, int nq, int d,
                  vector<T> &gt_density) {
  cout << "constructing QIE" << endl;
  vector<T> qi_density(nq);
  timer t;

  QIE qie(xs, nx, d);
  printf("construct QIE time \t%.5f\n", t.restart());
  for (int i = 0; i < nq; ++i) {
    qi_density[i] = qie.query(qs + i * d, 0.1);
  }
  printf("query QIE time \t%.5f\n", t.restart() / nq);
  printf("query variance \t%.5f\n",
         l2dist_sqr(gt_density.data(), qi_density.data(), nq));
}

void benchmark_rs(const T *xs, const T *qs, int nx, int nq, int d,
                  vector<T> &gt_density) {
  cout << "constructing RandomSample" << endl;
  vector<T> rs_density(nq);
  timer t;
  RandomSample rs(xs, nx, d);
  printf("construct RS time \t%.5f\n", t.restart());
  for (int i = 0; i < nq; ++i) {
    rs_density[i] = rs.query(qs + i * d, 64, 1024);
  }
  printf("query RS time \t%.5f\n", t.restart() / nq);
  printf("query variance \t%.5f\n",
         l2dist_sqr(gt_density.data(), rs_density.data(), nq));
}

void benchmark_hash(const T *xs, const T *qs, int nx, int nq, int d,
                    vector<T> &gt_density) {
  cout << "constructing HBE" << endl;
  vector<T> hb_density(nq);
  timer t;
  HBE lsh(xs, nx, d, 1, 16, 16, 6.0);
  printf("construct HBE time %.5f\n", t.restart());
  for (int i = 0; i < nq; ++i) {
    hb_density[i] = lsh.query(qs + i * d, 1, 16);
  }
  printf("query HBE time \t%.5f\n", t.restart());
  printf("query variance \t%.5f\n",
         l2dist_sqr(gt_density.data(), hb_density.data(), nq));
}
void benchmark(const T *xs, const T *qs, int nx, int nq, int d) {

  vector<T> gt_density(nq);
  benchmark_exact(xs, qs, nx, nq, d, gt_density);
  benchmark_rs(xs, qs, nx, nq, d, gt_density);
  benchmark_qi(xs, qs, nx, nq, d, gt_density);
}

int main() {
  int nx;
  int d;
  int nq = 100;
  vector<T> x;

//  const char *file = "../data/covtype.data";
//  load_csv<T>(&x, &nx, &d, file, 0, 1, ',');
//  T *xs = x.data();
//  T *qs = x.data();

//    const char *file = "/home/xydai/data/gas_sensor/gas_sensor_base.fvecs";
//    const char *file = "/home/xydai/data/nytimes-256/nytimes-256_base.fvecs";
    const char *file = "/home/xydai/data/sift-128/sift-128_base.fvecs";
    load_data<T, int>(&x, &nx, &d, file);
    T *xs = x.data();
    T *qs = x.data();

  //  nx = 1000000;
  //  d = 256;
  //  T *xs = random_normal(nx * d, 0, 0.1);
  //  T *qs = random_normal(nq * d, 0, 0.1);

  cout << "shape of " << nx << "x" << d << endl;
  benchmark(xs, qs, nx, nq, d);
}
