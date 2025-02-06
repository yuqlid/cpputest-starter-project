#include <array>
#include <cmath>
#include <fstream>
#include <iostream>

#include "CppUTest/TestHarness.h"
#include "current_controller.hpp"

extern "C" {
/*
 * Add your c-only include files here
 */
}

// #define _USE_MATH_DEFINES  // for C++

static constexpr float pi = 3.14159265358979323846264338327950288;
static constexpr float two_pi = 2.0f * pi;
static constexpr float fs_hz = 20000.0f;
static constexpr float bandwitdh = 300.0f;

TEST_GROUP(CurCtrl){void setup(){}

                    void teardown(){}};

TEST(CurCtrl, test1) {
  constexpr uint16_t period = 1024;

  CurrentControllerConfig_t config{2.5f};
  CurrentController cc(config);

  std::array<float, period> I_in_d;
  std::array<float, period> I_in_q;

  std::ofstream outfile("current_control.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }
  constexpr float gain = 3.0f;
  constexpr float omega = 9.5f;
  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    float amp = std::cos(two_pi * i / period);
    I_in_d.at(i) = gain * std::sin(omega * two_pi * i / period) * amp;
    I_in_q.at(i) = gain * std::cos(omega * two_pi * i / period) * amp;
  }

  outfile << "time,I_in/d,I_in/q,I_out/d,I_out/q" << std::endl;

  for (size_t i = 0; i < period; ++i) {
    cc.setTargetI(I_in_d.at(i), I_in_q.at(i));
    cc.update();
    directquadrature_t<float> I_out = cc.getOutput();
    outfile << static_cast<float>(i) / fs_hz << ", " << I_in_d.at(i) << ", "
            << I_in_q.at(i) << ", " << I_out.d << ", " << I_out.q << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored current_control.csv" << std::endl;
}
