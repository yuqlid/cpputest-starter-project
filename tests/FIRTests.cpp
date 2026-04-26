#include <cmath>
#include <fstream>
#include <iostream>

#include "CppUTest/TestHarness.h"
#include "fir.hpp"

extern "C" {
/*
 * Add your c-only include files here
 */
}

#define _USE_MATH_DEFINES  // for C++

TEST_GROUP(FIR){void setup(){}

                void teardown(){}};

TEST(FIR, test1) {
  constexpr uint16_t period = 256;
  float filter[10] = {-0.080441994f, -0.110027655, -0.003604771, 0.245479494,
                      0.46452132,    0.464521328,  0.245479494,  -0.003604771,
                      -0.110027655,  -0.080441994};

  FirDirect<float, 10> fir_direct(filter);
  FirTransposed<float, 10> fir_transposed(filter);
  float x[period];

  std::ofstream outfile("fir_test.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  for (int i = 0; i < period; ++i) {
    x[i] = 0.0f;
    if (i > 100) {
      x[i] = 1.0f;
    }
  }

  outfile << "in, out_direct, out_trans" << std::endl;

  for (size_t i = 0; i < period; ++i) {
    float y1 = fir_direct.update(x[i]);
    float y2 = fir_transposed.update(x[i]);
    outfile << x[i] << ", " << y1 << ", " << y2 << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored fir_test.csv" << std::endl;
}
