//
// Copyright (c) 2020 xinyan. All rights reserved.
// Created on 2020/3/31.
//

#ifndef SSKDE_INCLUDE_KDE_EXACT_H_
#define SSKDE_INCLUDE_KDE_EXACT_H_

#include <utils.h>
#include <cstring>

class ExactKDE {
 public:
  ExactKDE(const T* x, int n, int d): n_(n), d_(d) {
    x_ = new T[n * d];
    std::memcpy(x_, x, sizeof(T) * n * d);
  }
  ~ExactKDE() {
    delete[] x_;
  }
  T query(const T* q) const {
    T density = 0;
    T* x = x_;
    for (int i = 0; i < n_; ++i, x+=d_) {
      density += gaussian_kernel(q, x, 1, d_);
    }
    return density / n_;
  }
 private:
  T* x_;
  int n_;
  int d_;
};

#endif //SSKDE_INCLUDE_KDE_EXACT_H_
