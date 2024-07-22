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

TEST_GROUP(EncoderPLL){void setup(){}

                       void teardown(){}};

/**
 * @brief Construct a new TEST object
 * ステップ入力時の偏差のテスト
 */
TEST(EncoderPLL, step_response_delta_theta) {
  constexpr uint16_t period = 512;
  float x[period];
  constexpr float fs_hz = 20000.0f;
  constexpr float ts = 1 / fs_hz;
  constexpr float pi = 3.14159265358979323846264338327950288;
  constexpr float omega_pll = 2 * pi * 1000;
  constexpr float zeta_pll = 1.0f;
  constexpr float kp = 2 * zeta_pll * omega_pll;
  constexpr float ki = omega_pll * omega_pll;

  constexpr float num = 4 + 2 * kp * ts + ki * ts * ts;
  constexpr float b0 = 4 / num;
  constexpr float b1 = -2 * b0;
  constexpr float b2 = b0;

  constexpr float a1 = (8 - 2 * ki * ts * ts) / num;
  constexpr float a2 = -(4 - 2 * kp * ts + ki * ts * ts) / num;

  std::array<float, 2> ak = {a1, a2};
  std::array<float, 3> bk = {b0, b1, b2};
  Biquad<float> iir(ak, bk);

  std::ofstream outfile("step_response_delta_theta.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    // ステップ入力
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

  std::cout << "stored step_response_delta_theta.csv" << std::endl;
}

/**
 * @brief Construct a new TEST object
 * ステップ入力時の出力のテスト
 */
TEST(EncoderPLL, step_response_theta) {
  constexpr uint16_t period = 512;
  float x[period];
  constexpr float fs_hz = 20000.0f;
  constexpr float ts = 1 / fs_hz;
  constexpr float pi = 3.14159265358979323846264338327950288;
  constexpr float omega_pll = 2 * pi * 1000;
  constexpr float zeta_pll = 1.0f;
  constexpr float kp = 2 * zeta_pll * omega_pll;
  constexpr float ki = omega_pll * omega_pll;

  constexpr float num = 4 + 2 * kp * ts + ki * ts * ts;
  constexpr float b0 = (2 * kp * ts + ki * ts * ts) / num;
  constexpr float b1 = (2 * ki * ts * ts) / num;
  constexpr float b2 = (ki * ts * ts - 2 * kp * ts) / num;

  constexpr float a1 = -(2 * ki * ts * ts - 8) / num;
  constexpr float a2 = -(4 - 2 * kp * ts + ki * ts * ts) / num;

  std::array<float, 2> ak = {a1, a2};
  std::array<float, 3> bk = {b0, b1, b2};
  Biquad<float> iir(ak, bk);

  std::ofstream outfile("step_response_theta.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    // ステップ入力
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

  std::cout << "stored step_response_theta.csv" << std::endl;
}
