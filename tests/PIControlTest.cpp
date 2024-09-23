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
  constexpr float fc_hz = 10.0f;

  FirstOrderLpf<float> lpf_1(fs_hz, fc_hz);
  FirstOrderLpf<float> lpf_2(fs_hz, fc_hz);
  PIControllerConfig_t<float> PIConfig;

  constexpr float high = 2.0f;
  constexpr float low = -high;
  PIConfig.gain.kp = 2.0f;
  PIConfig.gain.ki = fs_hz * 0.2f;
  PIConfig.bandwitdh = 10.0f;  // テストに影響なし
  PIConfig.out_max = high;
  PIConfig.out_min = low;

  windup::PIController<float> cc_1(fs_hz);
  Integral_Anti_windup::PIController<float> cc_2(fs_hz);

  if (cc_1.isGainValid(PIConfig.gain)) {
    cc_1.setConfig(&PIConfig);
  } else {
    FAIL("Invalid Param");
  }
  if (cc_2.isGainValid(PIConfig.gain)) {
    cc_2.setConfig(&PIConfig);
  } else {
    FAIL("Invalid Param");
  }

  std::ofstream outfile("pi_test.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  constexpr uint16_t period = 512;
  std::array<float, period> ref;
  std::array<float, period> output_1;
  std::array<float, period> output_2;

  std::fill(output_1.begin(), output_1.end(), 0.0f);
  std::fill(output_2.begin(), output_2.end(), 0.0f);

  for (int i = 0; i < period; ++i) {
    ref.at(i) = 0.0f;
    if (i > 0) {
      ref.at(i) = 1.0f;
    }
    if (i > 250) {
      ref.at(i) = -1.0f;
    }
    if (i > 400) {
      ref.at(i) = 0.0f;
    }
  }

  outfile << "time,ref,input_1,out_1,input_2,out_2" << std::endl;

  for (size_t i = 0; i < period - 1; ++i) {
    float error_1 = ref.at(i) - output_1.at(i);
    float error_2 = ref.at(i) - output_2.at(i);
    float input_1 = cc_1.process(error_1);
    float input_2 = cc_2.process(error_2);

    float out_1 = lpf_1.update(input_1);
    float out_2 = lpf_2.update(input_2);

    output_1.at(i + 1) = out_1;
    output_2.at(i + 1) = out_2;
    /*
    outfile << static_cast<float>(i) / fs_hz << ", " << ref.at(i) << ", "
            << cc_1.getcaclP() << ", " << cc_1.getcalcI() << ", " << out_1
            << ", " << cc_2.getcaclP() << ", " << cc_2.getcalcI() << ", "
            << out_2 << ", " << std::endl;
    */
    outfile << static_cast<float>(i) / fs_hz << ", " << ref.at(i) << ", "
            << input_1 << ", " << out_1 << ", " << input_2 << ", "
            << out_2 << ", " << std::endl;
  }

  // ファイルを閉じる
  outfile.close();

  std::cout << "stored pi_test.csv" << std::endl;
}
