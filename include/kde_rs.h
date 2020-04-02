//
// Copyright (c) 2020 xinyan. All rights reserved.
// Created on 2020/3/31.
//

#ifndef SSKDE_INCLUDE_KDE_RS_H_
#define SSKDE_INCLUDE_KDE_RS_H_

#include <utils.h>
#include <cstring>
#include <vector>

class RandomSample {
 public:
  RandomSample(T* x, int n, int d): n_(n), d_(d) {
    x_ = new T[n * d];
    std::memcpy(x_, x, sizeof(T) * n * d);
  }

  T query(const T* q, int l, int m) const {
    std::random_device rd;
    auto rng = std::mt19937_64(rd());
    std::uniform_int_distribution<int > distribution(0, n_ - 1);
    std::vector<T > z = std::vector<T >(l, 0);
    for (int i = 0; i < l; i ++) {
      for (int j = 0; j < m; j ++) {
        z[i] += gaussian_kernel(q, x_ + distribution(rng) * d_, 1, d_);
      }
    }
    return median(&z) / m;
  }

 private:
  T* x_;
  int n_;
  int d_;
};

#endif  // SSKDE_INCLUDE_KDE_RS_H_
