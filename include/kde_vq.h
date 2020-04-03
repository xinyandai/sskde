//
// Copyright (c) 2020 xinyan. All rights reserved.
// Created on 2020/4/2.
//

#ifndef SSKDE_INCLUDE_KDE_VQ_H_
#define SSKDE_INCLUDE_KDE_VQ_H_

#include <progress_bar.h>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include <tuple>
#include <array>
#include <unordered_map>

#define Ks 256
using CodeType = uint8_t;

using std::array;
using std::unordered_map;

int vq(const T* w, const T* dict, int ks, int d) {
  int re = 0;
  T min_dist = l2dist_sqr(w, dict, d);
  for (int i = 1; i < ks; ++i) {
    dict += d;
    T dist = l2dist_sqr(w, dict, d);
    if (dist < min_dist) {
      re = i;
      min_dist = dist;
    }
  }
  return re;
}

/**
 * \param centroids [ks, sub_d]
 * \param code      [n]
 * \param data      [n, full_d]
 * \param n
 * \param k
 * \param sub_d
 * \param full_d
 * \param iter
 */
void kmeans(T* centroids, CodeType* code, const T* data,
            const int n, const int ks, const int sub_d,
            const int full_d, const int iter) {
  if (ks > n) {
    throw std::runtime_error("too many centroids");
  }
  std::uniform_int_distribution<> distribution(0, n-1);
  // random initialization
  std::memcpy(centroids, data, ks * sub_d * sizeof(T));

  ProgressBar bar(iter, std::string("k-means"));
  for (int i = 0; i < iter; ++i, ++bar) {
    // assign
#pragma omp parallel for
    for (int t = 0; t < n; ++t) {
      code[t] = static_cast<CodeType>(
          vq(&data[t * full_d], centroids, ks, sub_d));
    }

    // recenter
    vector<int > count(ks, 0);
    std::memset(centroids, 0, ks * sub_d * sizeof(T));
    for (int t = 0; t < n; ++t) {
      CodeType c = code[t];
      count[c]++;
      for (int dim = 0; dim < sub_d; ++dim) {
        centroids[c * sub_d + dim] += data[t * full_d + dim];
      }
    }

    for (int c = 0; c < ks; ++c) {
      if (count[c] == 0) {
        int t = distribution(rng);
        std::memcpy(&centroids[c * sub_d],
                    &data[t * full_d],
                    sub_d * sizeof(T));
      } else {
        for (int dim = 0; dim < sub_d; ++dim) {
          centroids[c * sub_d + dim] /= count[c];
        }
      }
    }
  }

}

template <size_t M=2>
class VQE {
 public:
  VQE(const T* x, int n, int d): n_(n), d_(d), dims_(M + 1, 0) {
    // dims_[0] = 0
    for (int i = 0; i < M; ++i) {
      dims_[i+1] = dims_[i] + d_ / M + (i < d_ % M) ? 1 : 0;
    }

    x_ = new T[n * d];
    dict_ = new T[Ks * dims_[1]];
    code_ = new CodeType[M * n];
    train_pq();
    std::memcpy(x_, x, sizeof(T) * n * d);
  }

  void train_pq() {
    const int sub_d = dims_[1];
    for (int i = 0; i < M; ++i) {
      kmeans(/*centroids*/ &dict_[i * Ks * sub_d],
             /*code*/      &code_[i * n_],
             /*data*/      x_ + dims_[i],
             /*n*/         n_,
             /*Ks*/        Ks,
             /*sub_d*/     dims_[i+1] - dims_[i],
             /*full_d*/    d_,
             /*iter*/      20);
    }
  }

  ~VQE() {
    delete[] x_;
    delete[] dict_;
    delete[] code_;
  }

  T query(const T* q, T dist) const {
    T table[M][Ks];
    const int sub_d = dims_[1];
    T* codeword = dict_;
    for (int m = 0; m < M; ++m) {
      for (int k = 0; k < Ks; ++k) {
        // &dict_[m * Ks * sub_d + k * sub_d]
        table[m][Ks] = l2dist_sqr(q + dims_[m], codeword,
                                  dims_[m+1] - dims_[m]);
        codeword += sub_d;
      }
    }

    T density = 0;
    T* x = x_;
    for (int i = 0; i < n_; ++i, x+=d_) {
      T dist_sqr = 0;
      for (int m = 0; m < M; ++m) {
        CodeType c = code_[m * n_+ i];
        dist_sqr += table[m][c];
      }
      if (dist_sqr < dist) {
        density += gaussian_kernel(q, x, 1, d_);
      } else {
        density += gaussian_kernel(dist_sqr, 1);
      }
    }
    return density / n_;
  }

 protected:
  const int n_;
  const int d_;

  T*           x_;      // n * d
  T*           dict_;   // M * Ks * dims_[1]
  CodeType*    code_;   // M * n
  vector<int > dims_;   // M + 1
};

#endif  // SSKDE_INCLUDE_KDE_VQ_H_
