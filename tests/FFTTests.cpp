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
constexpr float two_pi = 2.0f * pi;
constexpr uint32_t bit = 11;
constexpr uint32_t array_size = 1 << bit;  // 2^bit

// 入力波形
std::array<std::complex<double>, array_size> input;
std::array<double, array_size> input_re;
std::array<double, array_size> input_im;

TEST_GROUP(FFTTest){void setup(){for (uint32_t i = 0; i < array_size; i++){
    input.at(i) = std::complex<double>(std::sin(two_pi * i / array_size), 0.0f);
input_re.at(i) = input.at(i).real();
input_im.at(i) = input.at(i).imag();
}
}

void teardown() {}
}
;

TEST(FFTTest, fft1) {
  constexpr uint16_t period = array_size;

  //  書き込むファイル作成
  std::ofstream outfile("fft1.csv");

  // 入力兼出力用配列の作成
  std::array<double, period> re = input_re;
  std::array<double, period> im = input_im;

  // FFTの実行
  PaulBourke::fft(true, bit, re.data(), im.data());

  if (!outfile) {
    FAIL("file open failed");  // ファイルが開けないとテスト失敗とする
  }

  outfile << "i, input, Re, Im" << std::endl;
  for (int i = 0; i < period; ++i) {
    outfile << i << ", " << input_re.at(i) << "," << re.at(i) << ", "
            << im.at(i) << std::endl;
  }

  outfile.close();  // ファイルを閉じる

  std::cout << "stored reslut.csv" << std::endl;

  input_re = re;
  input_im = im;

  // FFTの実行
  PaulBourke::fft(false, bit, re.data(), im.data());

  //  書き込むファイル作成
  std::ofstream outfile1("fft1_inv.csv");
  if (!outfile1) {
    FAIL("file open failed");  // ファイルが開けないとテスト失敗とする
  }

  outfile1 << "i,In(Re),In(Im),Out(Re),Out(Im)" << std::endl;
  for (int i = 0; i < period; ++i) {
    outfile1 << i << ", " << input_re.at(i) << ", " << input_im.at(i) << ","
             << re.at(i) << ", " << im.at(i) << std::endl;
  }

  outfile1.close();  // ファイルを閉じる
}

TEST(FFTTest, fft2) {
  constexpr uint16_t period = array_size;
  //  書き込むファイルを開く
  std::ofstream outfile("fft2.csv");

  // 入力兼出力用配列の作成
  std::array<std::complex<double>, period> complexArray = input;

  // FFTの実行
  mikami::fft(complexArray.data(), period);

  if (!outfile) {
    FAIL("file open failed");  // ファイルが開けないとテスト失敗とする
  }

  outfile << "i, input, Re, Im" << std::endl;
  for (int i = 0; i < period; ++i) {
    outfile << i << ", " << input.at(i).real() << ","
            << complexArray.at(i).real() << ", " << complexArray.at(i).imag()
            << std::endl;
  }

  outfile.close();  // ファイルを閉じる

  std::cout << "stored reslut.csv" << std::endl;

  std::ofstream outfile1("fft2_inv.csv");
  input = complexArray;
  mikami::fft(complexArray.data(), -period);
  if (!outfile1) {
    FAIL("file open failed");  // ファイルが開けないとテスト失敗とする
  }

  outfile1 << "i, input, Re, Im" << std::endl;
  for (int i = 0; i < period; ++i) {
    outfile1 << i << ", " << input.at(i).real() << ","
             << complexArray.at(i).real() << ", " << complexArray.at(i).imag()
             << std::endl;
  }

  outfile1.close();  // ファイルを閉じる
}