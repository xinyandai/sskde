//
// Copyright (c) 2020 xinyan. All rights reserved.
// Created on 2020/4/2.
//

#ifndef SSKDE_INCLUDE_KDE_QIE_H_
#define SSKDE_INCLUDE_KDE_QIE_H_

#include <kde_vq.h>
#include <kde_lsh.h>
#include <unordered_map>

class QIE : public VQE<2 > {
 public:
  QIE(const T* x, int n, int d): VQE<2 >(x, n, d) {
    for (int i = 0; i < this->n_; ++i) {
      std::array<CodeType, 2> index = { this->code_[i], this->code_[n + i] };
      ht_[index].update(x + i * d_, d_);
    }
  }
  ~QIE() = default;

  T query(const T* q, const T dist) const {
    T table[2][Ks];
    int idx[2][Ks];

    const int sub_d = dims_[1];
    T* codeword = dict_;
    for (int m = 0; m < 2; ++m) {
      for (int k = 0; k < Ks; ++k) {
        table[m][Ks] = l2dist_sqr(q + dims_[m], codeword,
                                  dims_[m+1] - dims_[m]);
        codeword += sub_d;
      }
    }

#pragma unroll
    for (int i = 0; i < 2; ++i) {
      const T* table_i = table[i];
      std::iota(idx[i], idx[i]+Ks, 0);
      std::sort(
          idx[i], idx[i]+Ks,
          [table_i](size_t i1, size_t i2) {
            return table_i[i1] < table_i[i2];
          });
    }

    T density = 0;

    std::array<CodeType, 2> index = {0, 0};
    for (int i = 0; i < Ks; ++i) {
      T dist_i = table[0][ idx[0][i] ];
      if (dist_i + table[1][ idx[1][0] ] >= dist) {
        break;
      }

      index.at(0) = idx[0][i];

      for (int j = 0; j < Ks; ++j) {
        T dist_j = table[1][ idx[1][j] ];
        if (dist_i + dist_j >= dist) {
          break;
        }

        index.at(1) = idx[1][j];

        auto it = ht_.find(index);
        if (it != ht_.end()) {
          T ds = gaussian_kernel(q, it->second.data(), 1, d_);
          density +=  ds * it->second.size();
        }
      }
    }
    return density / this->n_;
  }

 private:
  unordered_map<array<CodeType, 2 >, HashBucket,
                VectorHash<std::array<CodeType, 2 > > > ht_;
};

#endif //SSKDE_INCLUDE_KDE_QIE_H_
