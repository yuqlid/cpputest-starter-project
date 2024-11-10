/**
 * @file trigonometric_funcTest.cpp
 * @author KUSAKABE Yuki (yuqlid@dgmail.com)
 * @brief
 * @version 0.1
 * @date 2024-06-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <array>
#include <fstream>
#include <iostream>

#include "CppUTest/TestHarness.h"
#include "fft.hpp"

extern "C" {
/*
 * Add your c-only include files here
 */
}
#define _USE_MATH_DEFINES  // for C++
#include <cmath>

constexpr float pi = 3.14159265358979323846264338327950288;

TEST_GROUP(FFTTest){void setup(){}

                    void teardown(){}};

TEST(FFTTest, fft1) {
  constexpr uint32_t bit = 11;
  constexpr uint16_t period = 2048;
  // uint16_t period = 2048;
  //  書き込むファイルを開く
  std::ofstream outfile("fft.csv");
  double in[period] = {0.0};
  double x[period] = {0.0};
  double y[period] = {0.0};
  constexpr float two_pi = 2.0f * pi;
  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    x[i] = std::cos(two_pi * i / period);
    in[i] = std::cos(two_pi * i / period);
  }

  FFT(true, bit, static_cast<double*>(x), static_cast<double*>(y));
  // ファイルが正しく開けたか確認する

  if (!outfile) {
    FAIL("file open failed");
  }
  outfile << "i, input, Re, Im" << std::endl;
  // forループで計算し、結果をファイルに書き込む
  for (int i = 0; i < period; ++i) {
    outfile << i << ", " << in[i] << "," << x[i] << ", " << y[i] << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored reslut.csv" << std::endl;
}

TEST(FFTTest, fft2) {
  // constexpr uint32_t bit = 11;
  constexpr uint16_t period = 2048;
  // uint16_t period = 2048;
  //  書き込むファイルを開く
  std::ofstream outfile("fft_complex.csv");
  // double in[period] = {0.0};
  double x[period] = {0.0};
  double y[period] = {0.0};
  //constexpr float two_pi = 2.0f * pi;
  constexpr float two_pi = 2.0f * static_cast<float>(M_PI);
  std::array<std::complex<double>, period> complexArray;
  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    x[i] = std::cos(two_pi * i / period);
    // in[i] = std::cos(two_pi * i / period);
    complexArray.at(i) = std::complex<double>(x[i], y[i]);
  }

  fft_dif(complexArray.data(), period);
  // ファイルが正しく開けたか確認する

  if (!outfile) {
    FAIL("file open failed");
  }
  outfile << "i, input, Re, Im" << std::endl;
  // forループで計算し、結果をファイルに書き込む
  for (int i = 0; i < period; ++i) {
    outfile << i << ", " << x[i] << "," << complexArray.at(i).real() << ", "
            << complexArray.at(i).imag() << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored reslut.csv" << std::endl;
}