//
// Copyright (c) 2020 xinyan. All rights reserved.
// Created on 2020/3/31.
//

#ifndef SSKDE_INCLUDE_LSH_KDE_H_
#define SSKDE_INCLUDE_LSH_KDE_H_

#include "utils.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>
#include <unordered_map>

using std::vector;
using std::unordered_map;

const T CONST_PI = 3.14159265358979323846264338327950288f;
const T SQRT_2PI = std::sqrt(2.0f / CONST_PI);

class HashBucket {
 public:
  /**
   * \brief default constructor
   */
  HashBucket() : count(0), x_() {}

  /**
   * \brief Build a new hash bucket for data point p.
   * \param p a data point that belongs to the bucket
   */
  HashBucket(const T* p, int d) : count(1), x_(p, p+d) {}

  /**
   * Insert point into bucket: update count and replace bucket sample
   * with reservoir sampling.
   * \brief
   * \param p
   * \param rng
   */
  void update(const T* p, int d) {
    count += 1;
    auto distribution = std::uniform_real_distribution<T>(0, 1);
    T r = distribution(rng);
    if (r <= 1.0 / count) {
      if (x_.empty()) {
        x_.resize(1ull * d);
      }
      std::memcpy(x_.data(), p, sizeof(T) * d);
    }
  }

  /**
   * Insert point into bucket: update count and append p to bucket
   * \brief
   * \param p
   * \param rng
   */
  void insert(T* p, int d) {
    count += 1;
    x_.resize(x_.size() + d);
    std::memcpy(x_.data() + x_.size() - d, p, sizeof(T) * d);
  }

  int size() const {
    return count;
  }

  const T* data() const {
    return x_.data();
  }

 private:
  int count;
  vector<T> x_;
};

class E2LSH {
 public:
  /**
   * \brief Construct an LSH table
   * \param xs dataset
   * \param n size of dataset
   * \param d dimension of dataset
   * \param k number of hash functions
   * \param w bin width
  */
  E2LSH(int d, int k, T w) : k(k), d(d), w(w) {
    a_ = random_normal(k * d);
    b_ = random_uniform(k, .0, w);
  }

  void init(const T* a, const T* b) {
    std::memcpy(a_, a, sizeof(T) * k * d);
    std::memcpy(b_, b, sizeof(T) * k);
  }

  ~E2LSH() {
    delete [] a_;
    delete [] b_;
  }

  vector<int > hash(const T* x) const {
    vector<int > v(k, 0);
    T* a = a_;
    T* b = b_;
    for (int i = 0; i < k; ++i, a+=d, b++) {
      T t = inner_product(x, a, d) + *b;
      v[i] = static_cast<int>(std::ceil(t / w));
    }
    return v;
  }

  T probability(T c) const {
    if (c <= 1e-10)
      return 1.f;
    c = c / w;
    T base = std::erf(1.0f / c) - SQRT_2PI * c *
             (1.0f - std::exp(-1.0f / (2 * c * c)));
    return static_cast<T >(std::pow(base, k));
  }

 public:
  const int d;
  const int k;
  const T w;

 private:
  T* a_;
  T* b_;
};

class HashTable {
 public:
  /**
   * \brief Construct an LSH table
   * \param xs dataset
   * \param n size of dataset
   * \param d dimension of dataset
   * \param k number of hash functions
   * \param w bin width
   */
  explicit HashTable(const E2LSH& lsh) : count_(0), lsh_(lsh) {}

  void insert(const T* x) {
    vector<int > key = lsh_.hash(x);
    auto it = table_.find(key);
    if (it == table_.end()) {
      table_[key] = HashBucket(x, lsh_.d);
      count_++;
    } else {
      it->second.update(x, lsh_.d);
    }
  }
  
  /**
   * \brief Find hash buckets that the query falls in. 
   * \param query  query point
   * \return 
   */
  T query(const T* q) const {
    vector<int > key = lsh_.hash(q);
    HashBucket bucket;
    const auto it = table_.find(key);
    if (it == table_.end()) {
      return 0.f;
    } else {
      T c_sqr = l2dist_sqr(q, it->second.data(), lsh_.d);
      return gaussian_kernel(c_sqr, 1.f) * it->second.size()
             / lsh_.probability(std::sqrt(c_sqr));
    }
  }

  int size() {
    return count_;
  }

 private:
  int count_;
  const E2LSH& lsh_;
  unordered_map<vector<int >, HashBucket,
                VectorHash<vector<int > > > table_;
};

class HBE {
 public:
  HBE(const T* xs, int n, int d, int l, int m, int k, T w)
             : l_(l), m_(m), n_(n), d_(d) {
    hash_.reserve(1ull * l * m);
    tables_.reserve(1ull * l * m);
    for (int i = 0; i < l * m; ++i) {
      hash_.emplace_back(d, k, w);
      tables_.emplace_back(hash_.back());
    }
#pragma omp parallel for
    for (int i = 0; i < l * m; ++i) {
      const T* x = xs;
      for (int j = 0; j < n; ++j, x+=d) {
        tables_[i].insert(x);
      }
    }
    std::cout << "table size : ";
    for (int i = 0; i < l * m; ++i) {
      std::cout << tables_[i].size() << "  ";
    }
    std::cout << std::endl;
  }

  T query(const T* q, int l, int m) const {
    l = std::min(l, l_);
    m = std::min(m, m_);
    std::vector<T > z = std::vector<T >(l, 0);
    int table_idx = 0;
    for (int i = 0; i < l; i ++) {
      for (int j = 0; j < m; j ++) {
        z[i] += tables_[table_idx++].query(q);
      }
    }
    return median(&z) / n_ / m;
  }

 private:
  int l_;
  int m_;
  int n_;
  int d_;
  vector<E2LSH >     hash_;
  vector<HashTable > tables_;
};

#endif  // SSKDE_INCLUDE_LSH_KDE_H_
