#include <cmath>
#include <fstream>
#include <iostream>

#include "CppUTest/TestHarness.h"
#include "thermistor.hpp"

extern "C" {
/*
 * Add your c-only include files here
 */
}

#define _USE_MATH_DEFINES  // for C++

TEST_GROUP(NTC){void setup(){}

                void teardown(){}};

TEST(NTC, test2) {
  NtcThermistor<4095> thermistor(10000.0f, 10000.0f, 3435.0f);

  uint16_t period = 4095;
  // 書き込むファイルを開く
  std::ofstream outfile("thermistor.csv");

  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }
  outfile << "adc, temp" << std::endl;
  // forループで計算し、結果をファイルに書き込む
  for (int i = 1; i < period; ++i) {
    outfile << i << ", " << thermistor.getTempraturDegC(i) << std::endl;
  }
  // ファイルを閉じる
  outfile.close();
  std::cout << "stored thermistor.csv" << std::endl;
}
