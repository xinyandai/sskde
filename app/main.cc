//
// Copyright (c) 2020 Xinyan. All rights reserved.
// Created on 2020/3/31.
//

#include <kde_exact.h>
#include <kde_rs.h>
#include <kde_lsh.h>
#include <data_loader.h>

#include <iostream>

using std::cout;
using std::endl;

int main() {
//  int n, d;
//  vector<float > x;
//  const char* file = "../data/covtype/covtype.data";
//  cout << "loading " << file << endl;
//  load(&x, &n, &d, 1, 0, 1);

  int n = 100000, d = 16;
  T* x_ptr = random_normal(n * d);
  T* q = x_ptr;

  cout << "shape of " << n << "x" << d << endl;
  cout << "constructing ExactKDE" << endl;
  ExactKDE kde(x_ptr, n, d);
  cout << "constructing RandomSample" << endl;
  RandomSample rs(x_ptr, n, d);
  cout << "constructing HBE" << endl;
  HBE lsh(x_ptr, n, d, 1, 16, 16, 8.0);
  cout << "Exact KDE" << endl;
  cout << kde.query(q) << endl;
  cout << "HBE" << endl;
  cout << lsh.query(q, 1, 1) << endl;
  cout << lsh.query(q, 1, 2) << endl;
  cout << lsh.query(q, 1, 4) << endl;
  cout << lsh.query(q, 1, 16) << endl;
  cout << "Random Sample" << endl;
  cout << rs.query(q, 1, 1u << 4u) << endl;
  cout << rs.query(q, 1, 1u << 8u) << endl;
  cout << rs.query(q, 1, 1u << 10u) << endl;
  cout << rs.query(q, 1, 1u << 16u) << endl;
  cout << rs.query(q, 1, 1u << 20u) << endl;
  cout << rs.query(q, 1, 1u << 24u) << endl;
}
