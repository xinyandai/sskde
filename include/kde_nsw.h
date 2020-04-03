//
// Created by xinyan on 3/4/2020.
//

#ifndef SSKDE_KDE_NSW_H
#define SSKDE_KDE_NSW_H

#include <hnswlib/hnswlib.h>
#include <hnswlib/space_l2.h>

using namespace hnswlib;

class GraphKDE {
 public:
  GraphKDE(const T* x, int n, int d): n_(n), d_(d), space_((size_t) d),
                                      hnsw_(&space_, (size_t) n, 16, 128) {
    hnsw_.addPoint((void *)x, (size_t) 0);
#pragma omp parallel for
    for (int i = 1; i < n; i++) {
      hnsw_.addPoint((void *) (x + d * i), (size_t) i);
    }

    hnsw_.setEf(64);
  }

  T query(const T* q, size_t k) {
    auto result = hnsw_.searchKnn(q, k);
    T density = 0.0;
    while (result.size()) {
      density += gaussian_kernel(result.top().first, 1.0f);
      result.pop();
    }
    return density / n_;
  }

 private:
  int n_;
  int d_;
  L2Space             space_;
  HierarchicalNSW<T > hnsw_;
};
#endif //SSKDE_KDE_NSW_H
