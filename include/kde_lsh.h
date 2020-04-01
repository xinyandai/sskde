//
// Copyright (c) 2020 xinyan. All rights reserved.
// Created on 2020/3/31.
//

#ifndef SSKDE_INCLUDE_LSH_KDE_H_
#define SSKDE_INCLUDE_LSH_KDE_H_

#include <algorithm>
#include <cmath>
#include <vector>
#include <unordered_map>

using std::vector;
using std::unordered_map;

const double CONST_PI = 3.14159265358979323846264338327950288;
const double SQRT_2PI = sqrt(2.0 / CONST_PI);

class HashBucket {
 public:
  /**
   * \brief default constructor
   */
  HashBucket() : count(0), x_() {
  }

  /**
   * \brief Build a new hash bucket for data point p.
   * \param p a data point that belongs to the bucket
   */
  HashBucket(T* p, int d) : count(1), x_(d) {
    std::memcpy(x_.data(), p, sizeof(T) * d);
  }

  /**
   * Insert point into bucket: update count and replace bucket sample
   * with reservoir sampling.
   * \brief
   * \param p
   * \param rng
   */
  void update(T* p, int d) {
    count += 1;
    std::random_device rd;
    auto rng = std::mt19937_64(rd());
    auto distribution = std::uniform_real_distribution<>(0, 1);
    float r = distribution(rng);
    if (r <= 1.0 / count) {
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
    std::memcpy(x_.data() - d, p, sizeof(T) * d);
  }

  int size() const {
    return count;
  }

  const T* data() const {
    return x_.data();
  };

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
    a_ = random_normal(k * d, .0, 1.0 / (w * w));
    b_ = random_uniform(k, .0, 1.0);
  }

  size_t hash(T* x) const {
    vector<T> v(b_, b_ + k);
    for (int i = 0; i < k; ++i) {
      v[i] += inner_product(x, &a_[i * d], d);
    }
    return hash_array(v.data(), k);
  }

  T probability(T c) const {
    if (c <= 1e-10)
      return 1.;
    c = c / w;
    T base = std::erf(1 / c) - SQRT_2PI * c *
             (1 - std::exp(-1.0f / (2 * c * c)));
    return std::pow(base, k);
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
  explicit HashTable(const E2LSH& lsh) : lsh_(lsh) {
  }

  void insert(T* x) {
    size_t key = lsh_.hash(x);
    auto it = table.find(key);
    if (it == table.end()) {
      table[key] = HashBucket(x, lsh_.d);
    } else {
      it->second.update(x, lsh_.d);
    }
  }
  
  /**
   * \brief Find hash buckets that the query falls in. 
   * \param query  query point
   * \return 
   */
  T query(T* q) const {
    size_t key = lsh_.hash(q);
    HashBucket bucket;
    const auto it = table.find(key);
    if (it == table.end()) {
      return 0.;
    } else {
      T c_sqr = l2dist_sqr(q, it->second.data(), lsh_.d);
      return gaussian_kernel(c_sqr, 1.) * it->second.size()
             / lsh_.probability(std::sqrt(c_sqr));
    }
  }

 private:
  const E2LSH& lsh_;
  unordered_map<size_t, HashBucket> table;
};

class LSHEstimater {
 public:
  LSHEstimater(T* xs, int n, int d, int l, int m, int k, T w): l_(l), m_(m), n_(n), d_(d) {
    hash_.reserve(l * m);
    tables_.reserve(l * m);
    for (int i = 0; i < l * m; ++i) {
      hash_.emplace_back(d, k, w);
      tables_.emplace_back(hash_.back());
    }
#pragma omp parallel for
    for (int i = 0; i < l * m; ++i) {
      T* x = xs;
      for (int j = 0; j < n; ++j, x+=d) {
        tables_[i].insert(x);
      }
    }
  }

  T query(T* q, int l, int m) const {
    l = std::min(l, l_);
    m = std::min(m, m_);
    std::vector<T > z = std::vector<T >(l, 0);
    int table_idx = 0;
    for (int i = 0; i < l; i ++) {
      for (int j = 0; j < m; j ++) {
        z[i] += tables_[table_idx++].query(q);
      }
    }
    return median(&z) / n_;
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
