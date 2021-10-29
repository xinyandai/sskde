//
// Copyright (c) 2020 xinyan. All rights reserved.
// Created on 2020/4/2.
//

#ifndef SSKDE_INCLUDE_PROGRESS_BAR_H_
#define SSKDE_INCLUDE_PROGRESS_BAR_H_

#pragma once
#include <iostream>
#include <string>

using std::string;

class ProgressBar {
public:
  ProgressBar(int len, const string &message, bool verbose)
      : len_(len), cur_(0), star_(0), verbose_(verbose) {
    if (!verbose_)
      return;
    std::cout << "0%   10   20   30   40   50   60   70   80   90   100%\t"
              << message << std::endl
              << "|----|----|----|----|----|----|----|----|----|----|"
              << std::endl;
  }

  ProgressBar &update(int i) {
    cur_ += i;
    if (!verbose_)
      return *this;
    int num_star = static_cast<int>(1.0 * cur_ / len_ * 50 + 1);
    if (num_star > star_) {
      for (int j = 0; j < num_star - star_; ++j) {
        std::cout << '*';
      }
      star_ = num_star;
      if (num_star == 51) {
        std::cout << std::endl;
      }
      std::cout << std::flush;
    }

    return *this;
  }

  ProgressBar &operator++() { return update(1); }

  ProgressBar &operator+=(int i) { return update(i); }

private:
  int len_;
  int cur_;
  int star_;
  bool verbose_;
};

#endif // SSKDE_INCLUDE_PROGRESS_BAR_H_
