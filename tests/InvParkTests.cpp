/**
 * @file InvParkTests.cpp
 * @author KUSAKABE Yuki (yuqlid@dgmail.com)
 * @brief
 * @version 0.1
 * @date 2024-07-30
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <fstream>
#include <iostream>

#include "CppUTest/TestHarness.h"
#include "invPark.hpp"
extern "C" {
/*
 * Add your c-only include files here
 */
}

// #define _USE_MATH_DEFINES  // for C++
constexpr float pi = 3.14159265358979323846264338327950288;

TEST_GROUP(InvPark){void setup(){}

                    void teardown(){}};

TEST(InvPark, test2) {
  uint16_t period = 2048;
  // 書き込むファイルを開く
  std::ofstream outfile("invpark_result.csv");

  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }
  outfile << "theta, V_d, V_q, V_alpha, V_beta" << std::endl;
  // forループで計算し、結果をファイルに書き込む
  for (int i = 0; i < period; ++i) {
    float theta = 2 * pi * i / period;
    float Vq = 1.0f;
    float Vd = 0.0f;
    float Va, Vb;

    invPark<float>(theta, Vd, Vq, Va, Vb);

    outfile << theta << ", " << Vd << ", " << Vq << ", " << Va << ", " << Vb
            << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored invpark_result.csv" << std::endl;
}
