/**
 * @file SinCosTest.cpp
 * @author KUSAKABE Yuki (yuqlid@dgmail.com)
 * @brief
 * @version 0.1
 * @date 2024-06-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <cmath>
#include <fstream>
#include <string>

#include "CppUTest/TestHarness.h"
#include "TestOutputPath.hpp"
#include "sincos_approx.hpp"

TEST_GROUP(SinCos) {
  const char *csv_file_name = nullptr;

  void teardown() {
    if (csv_file_name != nullptr) {
      TestOutputPath::copyToLatest("SinCos", csv_file_name);
    }
  }
};

namespace {
constexpr float kPi = 3.14159265358979323846264338327950288f;
constexpr float kTolerance = 0.002f;

void writeSinCosCsv(const std::string &file_name) {
  constexpr int period = 2048;
  const auto path = TestOutputPath::makeCsvPath("SinCos", file_name);
  std::ofstream csv(path);

  CHECK_TEXT(csv.is_open(), "sincos csv open failed");

  csv << "theta,std_sin,approx_sin,sin_error,std_cos,approx_cos,cos_error\n";

  for (int i = 0; i <= period; ++i) {
    const float theta = 2.0f * kPi * static_cast<float>(i) /
                        static_cast<float>(period);
    const float expected_sin = std::sin(theta);
    const float expected_cos = std::cos(theta);
    float approx_sin = 0.0f;
    float approx_cos = 0.0f;
    nick_sincos::sincos(theta, approx_sin, approx_cos);

    csv << theta << "," << expected_sin << "," << approx_sin << ","
        << approx_sin - expected_sin << "," << expected_cos << ","
        << approx_cos << "," << approx_cos - expected_cos << "\n";
  }
}
}  // namespace

TEST(SinCos, ApproximationStaysCloseToStdSinCos) {
  constexpr int period = 2048;

  for (int i = 0; i <= period; ++i) {
    const float theta = 2.0f * kPi * static_cast<float>(i) /
                        static_cast<float>(period);
    float approx_sin = 0.0f;
    float approx_cos = 0.0f;
    nick_sincos::sincos(theta, approx_sin, approx_cos);

    DOUBLES_EQUAL(std::sin(theta), approx_sin, kTolerance);
    DOUBLES_EQUAL(std::cos(theta), approx_cos, kTolerance);
  }
}

TEST(SinCos, SincosMatchesSeparateSinCos) {
  constexpr int period = 64;

  for (int i = 0; i <= period; ++i) {
    const float theta = 2.0f * kPi * static_cast<float>(i) /
                        static_cast<float>(period);
    float approx_sin = 0.0f;
    float approx_cos = 0.0f;
    nick_sincos::sincos(theta, approx_sin, approx_cos);

    DOUBLES_EQUAL(nick_sincos::sin(theta), approx_sin, 0.0);
    DOUBLES_EQUAL(nick_sincos::cos(theta), approx_cos, 0.0);
  }
}

TEST(SinCos, GenerateCsvForVisualInspection) {
  csv_file_name = "sincos_result.csv";
  writeSinCosCsv(csv_file_name);
}
