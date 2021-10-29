//
// Copyright (c) 2020 Xinyan. All rights reserved.
// Created on 2020/3/31.
//

#ifndef SSKDE_INCLUDE_DATA_LOADER_H_
#define SSKDE_INCLUDE_DATA_LOADER_H_

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>

using std::cerr;
using std::cout;
using std::string;
using std::vector;

template <typename T>
void load_csv(vector<T> *csv_array, int *n, int *d, const char *url,
              const int skip_row, const int skip_col, const char split) {
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
      try {
        T num = std::stod(val);
        csv_array->push_back(num);
        (*d)++;
      } catch (std::invalid_argument &e1) {
        /// do nothing
      }
    }
    (*n)++;
  }
}


template <typename T, typename int_t>
void load_data(vector<T> *x, int_t *n, int_t* d, const std::string& input_path)
{
  std::ifstream fin(input_path.c_str(), std::ios::binary | std::ios::ate);
  if (!fin) {
    std::cout << "cannot open file " << input_path << std::endl;
    exit(1);
  }

  size_t fileSize = fin.tellg();
  fin.seekg(0, fin.beg);
  if (fileSize == 0) {
    std::cout << "file size is 0 " << input_path << std::endl;
    exit(1);
  }

  int dim;
  fin.read(reinterpret_cast<char*>(&dim), sizeof(int));
  *d = (size_t)dim;
  size_t bytesPerRecord = dim * sizeof(T) + 4;
  if (fileSize % bytesPerRecord != 0) {
    std::cout << "File not aligned" << std::endl;
    exit(1);
  }
  *n = fileSize / bytesPerRecord;
  x->resize((*n) * dim);
  fin.read((char*)x->data(), sizeof(T) * dim);

  for (int i = 1; i < *n; ++i) {
    fin.read((char*)&dim, 4);
    assert(dim == (*d));
    fin.read((char*)(x->data() + i * dim), sizeof(T) * dim);
  }
  fin.close();
}
#endif // SSKDE_INCLUDE_DATA_LOADER_H_