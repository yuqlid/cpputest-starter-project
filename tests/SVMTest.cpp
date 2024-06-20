#include <cmath>
#include <fstream>
#include <iostream>

#include "CppUTest/TestHarness.h"
#include "invClarke.hpp"
#include "svm.hpp"

extern "C" {
/*
 * Add your c-only include files here
 */
}

#define _USE_MATH_DEFINES  // for C++

TEST_GROUP(SVM){void setup(){}

                void teardown(){}};

TEST(SVM, test1) {
  float a = 0.0;
  float b = sqrt(3.0) / 2.0;

  auto [u, v, w, res] = svm::calcDuty(a, b);
/*
  std::cout << u << std::endl;
  std::cout << v << std::endl;
  std::cout << w << std::endl;
  std::cout << res << std::endl;
*/
  CHECK(res);
  CHECK(u <= 1);
  CHECK(v <= 1);
  CHECK(w <= 1);
  /*
   * Instantiate your class, or call the function, you want to test.
   * Then delete this comment
   */
}

TEST(SVM, test2) {
  constexpr double pi = 3.14159265358979323846264338327950288;
  constexpr float amp = std::sqrt(3) / 2;

  uint16_t period = 2048;
  // 書き込むファイルを開く
  std::ofstream outfile("result.csv");

  // ファイルが正しく開けたか確認する
  if (!outfile) {
    std::cerr << "file open failed" << std::endl;
    // return 1; // エラーを示す
  }
  outfile << "theta, a, b, svm_u, svm_v, svm_w , u, v, w, svm_uv, svm_vw, "
             "svm_wu, uv, vw, wu"
          << std::endl;
  // forループで計算し、結果をファイルに書き込む
  for (int i = 0; i < period; ++i) {
    float a = amp * std::cos(2 * pi * i / period);
    float b = amp * std::sin(2 * pi * i / period);
    auto [svm_u, svm_v, svm_w, res] = svm::calcDuty(a, b);
    auto [u, v, w] = calcDuty(a, b);
    outfile << i << ", " << a << ", " << b << ", " << svm_u << ", " << svm_v
            << ", " << svm_w << ", " << u << ", " << v << ", " << w << ", "
            << svm_u - svm_v << ", " << svm_v - svm_w << ", " << svm_w - svm_u
            << ", " << u - v << ", " << v - w << ", " << w - u << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored reslut.csv" << std::endl;
}
