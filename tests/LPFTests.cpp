#include <array>
#include <cmath>
#include <complex>
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

TEST(LPF, step_response) {
  constexpr float fs_hz = 20000.0f;
  constexpr float fc_hz = 1000.0f;
  FirstOrderLpf<float> lpf(fs_hz, fc_hz);

  std::ofstream outfile("lpf_test.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  // 入力波形の時系列データ作成
  constexpr uint16_t period = 256;
  std::array<uint16_t, period> x;
  for (int i = 0; i < period; ++i) {
    x.at(i) = 0.0f;
    if (i > 100) {
      x.at(i) = 1.0f;
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

TEST(LPF, step_response2) {
  constexpr float fs_hz = 20000.0f;
  constexpr float fc_hz = 1000.0f;
  FirstOrderLpf<float> lpf1(fs_hz, fc_hz);
  FirstOrderLpf<float> lpf2(2.0f * fc_hz / fs_hz);

  std::ofstream outfile("lpf_test2.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  // 入力波形の時系列データ作成
  constexpr uint16_t period = 256;
  std::array<uint16_t, period> x;
  for (int i = 0; i < period; ++i) {
    x.at(i) = 0.0f;
    if (i > 100) {
      x.at(i) = 1.0f;
    }
  }

  outfile << "tims,in,out1,out2" << std::endl;

  for (size_t i = 0; i < period; ++i) {
    outfile << static_cast<float>(i) / fs_hz << ", " << x[i] << ", "
            << lpf1.update(x[i]) << ", " << lpf2.update(x[i]) << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored lpf_test2.csv" << std::endl;
}

TEST(LPF, transfer_function) {
  constexpr float pi = 3.14159265358979323846264338327950288;

  constexpr float fs_hz = 20000.0f;  //!< サンプリング周波数　unit : Hz
  constexpr float fc_hz = 2000.0f;   //!< カットオフ周波数　unit : Hz
  constexpr uint32_t omega_rads = fs_hz / 2 * 2 * pi;  //!< unit : rad/s

  constexpr std::complex<float> ts(1.0f / fs_hz, 0.0f);
  constexpr std::complex<float> two_tc(2.0f / (2.0f * pi * fc_hz), 0.0f);

  // 一次ローパスフィルタ係数　分母
  std::complex<float> a0(1.0f, 0.0f);
  std::complex<float> a1((ts - two_tc) / (ts + two_tc));

  // 一次ローパスフィルタ係数　分子
  std::complex<float> b0(ts / (two_tc + ts));
  std::complex<float> b1(b0);

  std::ofstream outfile("lpf_tf.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }
  outfile << "f[Hz],gain[dB],phase[deg]" << std::endl;

  // forループで計算し、結果をファイルに書き込む
  for (uint32_t i = 1; i < omega_rads; ++i) {
    /**
     * @brief z^-1 = exp(-j*omega*Ts)を計算する
     */
    std::complex<float> jtheta(0.0f, static_cast<float>(i) / fs_hz);
    std::complex<float> inv_z(std::exp(-jtheta));

    std::complex<float> lpf((b0 + b1 * inv_z) / (a0 + a1 * inv_z));
    /**
     * @brief 周波数応答を計算する。
     *
     */
    outfile << static_cast<float>(i) / (2.0f * pi)
            << ", "  //!< 周波数　unit : Hz
            << 20 * std::log10(std::abs(lpf))
            << ", "  //!< 振幅　20*log　unit : dB
            << 180.0 * std::arg(lpf) / pi
            << std::endl;  //!< 位相　unit : degree
  }

  // ファイルを閉じる
  outfile.close();

  std::cout << "stored lpf_tf.csv" << std::endl;
}
