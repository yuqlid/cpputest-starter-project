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
#include <cmath>
#include <fstream>
#include <iostream>

#include "CppUTest/TestHarness.h"
#include "trigonometric_func.hpp"
extern "C" {
/*
 * Add your c-only include files here
 */
}

// #define _USE_MATH_DEFINES  // for C++
constexpr float pi = 3.14159265358979323846264338327950288;

TEST_GROUP(SinCos){void setup(){}

                   void teardown(){}};

TEST(SinCos, test1) {
  auto [sin, cos] = nick_sincos::sincos(static_cast<float>(pi / 2));
  std::cout << sin << std::endl;
  CHECK(sin <= 1);
}

TEST(SinCos, test2) {
  uint16_t period = 2048;
  // 書き込むファイルを開く
  std::ofstream outfile("sincos_result.csv");

  // ファイルが正しく開けたか確認する
  if (!outfile) {
    std::cerr << "file open failed" << std::endl;
    // return 1; // エラーを示す
  }
  outfile << "theta, sin(theta), cos(theta)" << std::endl;
  // forループで計算し、結果をファイルに書き込む
  for (int i = 0; i < period; ++i) {
    float theta = 2 * pi * i / period;
    //float sin_theta = nick_sincos::sin(theta);
    //float cos_theta = nick_sincos::cos(theta);
    auto[sin_theta, cos_theta] = nick_sincos::sincos(theta);
    outfile << theta << ", " << sin_theta << ", " << cos_theta << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored reslut.csv" << std::endl;
}
