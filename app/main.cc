//
// Copyright (c) 2020 Xinyan. All rights reserved.
// Created on 2020/3/31.
//
#include <stdio.h>
#include <kde_exact.h>
#include <kde_qie.h>
#include <kde_rs.h>
#include <kde_nsw.h>
#include <kde_vq.h>
#include <kde_lsh.h>
#include <data_loader.h>

#include <iostream>

using std::cout;
using std::endl;


void test(const T* xs, const T* qs, const int nx, const int nq, const int d) {
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
    const T* q = qs + i * d;

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



void benchmark(const T* xs, const T* qs, int nx, int nq, int d) {
  cout << "constructing ExactKDE" << endl;
  vector<T > gt_density(nq);
  vector<T > vq_density(nq);
  vector<T > qi_density(nq);
  vector<T > hb_density(nq);
  vector<T > rs_density(nq);
  timer t;
  {
    t.restart();
    ExactKDE kde(xs, nx, d);
    printf("construct ExactKDE time \t%.5f\n", t.restart());
    for (int i = 0; i < nq; ++i) {
      gt_density[i] =  kde.query(qs + i * d);
    }
    printf("query ExactKDE time \t%.5f\n", t.restart());
  }

  {
    t.restart();
    GraphKDE graph(xs, nx, d);
    printf("construct HNSW time \t%.5f\n", t.restart());
    for (int i = 0; i < nq; ++i) {
      vq_density[i] =  graph.query(qs + i * d, 32);
    }
    printf("query HNSW time \t%.5f\n", t.restart());
    printf("query variance \t%.5f\n",
           l2dist_sqr(gt_density.data(), vq_density.data(), nq));
  }

  {
    t.restart();
    VQE<2> vqe(xs, nx, d);
    printf("construct VQE time \t%.5f\n", t.restart());
    for (int i = 0; i < nq; ++i) {
      vq_density[i] =  vqe.query(qs + i * d, 0.1);
    }
    printf("query VQE time \t%.5f\n", t.restart());
    printf("query variance \t%.5f\n",
           l2dist_sqr(gt_density.data(), vq_density.data(), nq));
  }

  {
    t.restart();
    QIE qie(xs, nx, d);
    printf("construct QIE time \t%.5f\n", t.restart());
    for (int i = 0; i < nq; ++i) {
      qi_density[i] =  qie.query(qs + i * d, 0.1);
    }
    printf("query QIE time \t%.5f\n", t.restart());
    printf("query variance \t%.5f\n",
           l2dist_sqr(gt_density.data(), qi_density.data(), nq));
  }
  
  {
    t.restart();
    RandomSample rs(xs, nx, d);
    printf("construct RS time \t%.5f\n", t.restart());
    for (int i = 0; i < nq; ++i) {
      rs_density[i] =  rs.query(qs + i * d, 1, 1024);
    }
    printf("query RS time \t%.5f\n", t.restart());
    printf("query variance \t%.5f\n",
           l2dist_sqr(gt_density.data(), rs_density.data(), nq));
  }
  
  {
    t.restart();
    HBE lsh(xs, nx, d, 1, 16, 16, 6.0);
    printf("construct HBE time %.5f\n", t.restart());
    for (int i = 0; i < nq; ++i) {
      hb_density[i] =  lsh.query(qs + i * d, 1, 16);
    }
    printf("query HBE time \t%.5f\n", t.restart());
    printf("query variance \t%.5f\n",
           l2dist_sqr(gt_density.data(), hb_density.data(), nq));
  }
}

int main() {
  int nx, nq, d;
  vector<T > x;

//  const char* file = "../data/covtype/covtype.data";
//  load<T>(&x, &nx, &d, file, 0, 1, ',');
//  T* x_ptr = x.data();
//  T* q_ptr = x.data();
//  nq = nx;

//  const char* train_file = "../data/shuttle/shuttle.trn";
//  const char* test_file = "../data/shuttle/shuttle.trn";
//  load<T>(&x, &nx, &d, train_file, 0, 1, ' ');
//  load<T>(&x, &nq, &d, test_file, 0, 1, ' ');
//  T* q_ptr = x.data();
//  T* x_ptr = x.data();

  nx = 1000000;
  nq = 10;
  d = 256;
  T* xs = random_normal(nx * d, 0, 0.1);
  T* qs = random_normal(nq * d, 0, 0.1);

  cout << "shape of " << nx << "x" << d << endl;
  benchmark(xs, qs, nx, nq, d);
}
