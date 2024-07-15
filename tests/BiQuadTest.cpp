#include <cmath>
#include <complex>
#include <fstream>
#include <iostream>

#include "CppUTest/TestHarness.h"
#include "iir.hpp"

extern "C" {
/*
 * Add your c-only include files here
 */
}

#define _USE_MATH_DEFINES  // for C++

TEST_GROUP(BiQuad){void setup(){}

                   void teardown(){}};

TEST(BiQuad, step_response) {
  constexpr uint16_t period = 512;
  float x[period];
  constexpr float fs_hz = 2000.0f;
  constexpr float ts = 1 / fs_hz;
  constexpr float pi = 3.14159265358979323846264338327950288;
  constexpr float omega_pll = 2 * pi * 10;
  constexpr float zeta_pll = 0.5f;

  constexpr float num = 1 + zeta_pll * omega_pll * ts +
                        ((omega_pll * ts / 2) * (omega_pll * ts / 2));
  constexpr float b0 = ((omega_pll * ts / 2) * (omega_pll * ts / 2)) / num;
  constexpr float b1 = 2 * b0;
  constexpr float b2 = b0;

  constexpr float a1 =
      2 * (1 - ((omega_pll * ts / 2) * (omega_pll * ts / 2))) / num;
  constexpr float a2 = -(1 - zeta_pll * omega_pll * ts +
                         ((omega_pll * ts / 2) * (omega_pll * ts / 2))) /
                       num;

  std::array<float, 2> ak = {a1, a2};
  std::array<float, 3> bk = {b0, b1, b2};
  Biquad<float> iir(ak, bk);

  std::ofstream outfile("biquad_iir.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    x[i] = 0.0f;
    if (i > 100) {
      x[i] = 1.0f;
    }
  }

  outfile << "tims,in,out" << std::endl;

  for (size_t i = 0; i < period; ++i) {
    outfile << static_cast<float>(i) / fs_hz << ", " << x[i] << ", "
            << iir.update(x[i]) << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored lpf_test.csv" << std::endl;
}
