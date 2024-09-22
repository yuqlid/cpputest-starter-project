/**
 * @file PIControlTest.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-09-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>

#include "CppUTest/TestHarness.h"
#include "PIController.hpp"
#include "firstorderlpf.hpp"

extern "C" {
/*
 * Add your c-only include files here
 */
}

#define _USE_MATH_DEFINES  // for C++

TEST_GROUP(PI){void setup(){}

               void teardown(){}};

TEST(PI, test1) {
  constexpr float fs_hz = 5000.0f;
  constexpr float fc_hz = 20.0f;

  FirstOrderLpf<float> lpf(fs_hz, fc_hz);
  PIControllerConfig_t<float> PIConfig;

  PIConfig.gain.kp = 2.0f;
  PIConfig.gain.ki = fs_hz *0.2f;
  PIConfig.bandwitdh = 10.0f;  // テストに影響なし

  constexpr float high = 2.0f;
  constexpr float low = -high;

  PIController<float> CurrentController(fs_hz);

  if (CurrentController.isGainValid(PIConfig.gain)) {
    CurrentController.setConfig(&PIConfig);
  } else {
    FAIL("Invalid Param");
  }

  std::ofstream outfile("pi_test.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }
  constexpr uint16_t period = 256;
  std::array<float, period> x;
  std::array<float, period> output;

  for (int i = 0; i < period; ++i) {
    x.at(i) = 0.0f;
    output.at(i) = 0.0f;
    if (i > 0) {
      x.at(i) = 1.0f;
    }
  }

  outfile << "time,ref,input,p,input_clamp,output" << std::endl;

  for (size_t i = 0; i < period - 1; ++i) {
    float error = x.at(i) - output.at(i);
    float input = CurrentController.process(error);
    float p = PIConfig.gain.kp * error;
    float input_clamp = std::clamp(input, low, high);
    float out = lpf.update(input_clamp);

    output.at(i + 1) = out;
    outfile << static_cast<float>(i) / fs_hz << ", " << x.at(i) << ", " << input
            << ", " << p << ", " << input_clamp << ", " << out << std::endl;
  }

  // ファイルを閉じる
  outfile.close();

  std::cout << "stored pi_test.csv" << std::endl;
}
