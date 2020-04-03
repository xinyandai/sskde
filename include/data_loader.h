//
// Copyright (c) 2020 Xinyan. All rights reserved.
// Created on 2020/3/31.
//

#ifndef SSKDE_INCLUDE_DATA_LOADER_H_
#define SSKDE_INCLUDE_DATA_LOADER_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using std::cout;
using std::cerr;
using std::string;
using std::vector;

template <typename T>
void load(vector<T >* csv_array, int * n, int * d,
          const char* url, const int skip_row,
          const int skip_col, const char split) {
  std::ifstream f;

  f.open(url);
  if (!f.is_open()) {
    cerr << "error: file open failed '" << url << "'.\n";
    return;
  }

  *n = 0;

  std::string line, val;

  for (int i = 0; i < skip_row; ++i) {
    std::getline(f, line);
  }

  while (std::getline(f, line)) {
    std::stringstream s(line);
    *d = 0;

    for (int i = 0; i < skip_col; ++i) {
      getline(s, val, split);
    }

    while (getline(s, val, split)) {
      csv_array->push_back(std::stod(val));
      (*d)++;
    }
    (*n)++;
  }
}
#endif //SSKDE_INCLUDE_DATA_LOADER_H_