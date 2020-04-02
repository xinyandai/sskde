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
  int n, d;
  const char* file = "../data/covtype/covtype.data";
  vector<float > x;
  cout << "loading " << file << endl;
  load(&x, &n, &d, file, 0, 1);

  cout << "shape of " << n << "x" << d << endl;
  cout << "constructing ExactKDE" << endl;
  ExactKDE kde(x.data(), n, d);
  cout << "constructing RandomSample" << endl;
  RandomSample rs(x.data(), n, d);
  cout << "constructing HBE" << endl;
  HBE lsh(x.data(), n, d, 1, 16, 16, 256.0);
  cout << "Exact KDE" << endl;
  cout << kde.query(x.data()) << endl;
  cout << "HBE" << endl;
  cout << lsh.query(x.data(), 1, 1) << endl;
  cout << lsh.query(x.data(), 2, 2) << endl;
  cout << lsh.query(x.data(), 4, 4) << endl;
  cout << lsh.query(x.data(), 16, 16) << endl;
  cout << "Random Sample" << endl;
  cout << rs.query(x.data(), 16, 16) << endl;
  cout << rs.query(x.data(), 32, 32) << endl;
  cout << rs.query(x.data(), 64, 64) << endl;
  cout << rs.query(x.data(), 128, 128) << endl;
  cout << rs.query(x.data(), 256, 256) << endl;
  cout << rs.query(x.data(), 1024, 1024) << endl;
}
