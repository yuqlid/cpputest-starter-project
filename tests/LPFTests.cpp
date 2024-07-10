#include <cmath>
#include <fstream>
#include <iostream>

#include "CppUTest/TestHarness.h"
#include "firstorderlpf.hpp"

extern "C" {
/*
 * Add your c-only include files here
 */
}

#define _USE_MATH_DEFINES  // for C++

TEST_GROUP(LPF){void setup(){}

                void teardown(){}};

TEST(LPF, test1) {
  constexpr uint16_t period = 256;
  float x[period];
  constexpr float fs_hz = 20000.0f;
  constexpr float fc_hz = 1000.0f;
  FirstOrderLpf<float> lpf(fs_hz, fc_hz);

  std::ofstream outfile("lpf_test.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    x[i] = 0.0f;
    if (i > 10) {
      x[i] = 1.0f;
    }
  }

  outfile << "tims,in,out" << std::endl;

  for (size_t i = 0; i < period; ++i) {
    outfile << static_cast<float>(i) / fs_hz << ", " << x[i] << ", "
            << lpf.update(x[i]) << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored lpf_test.csv" << std::endl;
}
