//
// Copyright (c) 2020 Xinyan. All rights reserved.
// Created on 2020/3/31.
//

#ifndef SSKDE_INCLUDE_UTILS_H_
#define SSKDE_INCLUDE_UTILS_H_

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <cmath>
#include <vector>

using T = float;
using std::vector;

/**initialized in utils.cc**/
extern std::random_device rd;
extern std::mt19937_64 rng;

static T l2dist_sqr(const T *vec1, const T *vec2, const int dim) {
  T dist = 0;
  for (int i = 0; i < dim; ++i) {
    T diff = *(vec1++) - *(vec2++);
    dist += diff * diff;
  }
  return dist;
}


static T inner_product(const T *vec1, const T *vec2, const int dim) {
  T ip = 0;
  for (int i = 0; i < dim; ++i) {
    ip += *(vec1++) * *(vec2++);
  }
  return ip;
}


static T gaussian_kernel(const T *vec1, const T *vec2,
                         const T bandwidth, const int dim) {
  return std::exp(-l2dist_sqr(vec1, vec2, dim) / bandwidth);
}

static T gaussian_kernel(const T c, const T bandwidth) {
  return std::exp(-c / bandwidth);
}


static T median(std::vector<T >* z) {
  size_t L = z->size();
  if (L == 1) { return (*z)[0]; }
  std::sort(z->begin(), z->end());
  return L % 2 ? (*z)[L / 2] : ((*z)[L / 2 - 1] + (*z)[L / 2]) / 2;
}


static T* random_normal(int n, T mean = 0.0, T var = 1.0) {
  T *r = new T[n];
  std::normal_distribution<T > normal(mean, var);
  for (int i = 0; i < n; i ++) {
    r[i] = normal(rng);
  }
  return r;
}

static T* random_uniform(int n, T lower = 0.0, T upper = 1.0) {
  T *r = new T[n];
  std::uniform_real_distribution<T > uniform(lower, upper);
  for (int i = 0; i < n; i ++) {
    r[i] = uniform(rng);
  }
  return r;
}


struct VectorHash {
  int operator()(const vector<int> &v) const {
    int seed = 0;
    for (int ele : v) {
      seed ^= std::hash<int >{}(ele) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

#endif  //SSKDE_INCLUDE_UTILS_H_

