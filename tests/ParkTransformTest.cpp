/**
 * @file ParkTransformTest.cpp
 * @author KUSAKABE Yuki (yuqlid@dgmail.com)
 * @brief
 * @version 0.1
 * @date 2026-05-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "CppUTest/TestHarness.h"
#include "inv_park.hpp"
#include "park.hpp"

TEST_GROUP(ParkTransform) {};

namespace {
constexpr float kPi = 3.14159265358979323846264338327950288f;
constexpr float kTolerance = 0.005f;

struct Vector2 {
  float a;
  float b;
};

constexpr float kAngles[] = {
    -kPi,
    -2.0f * kPi / 3.0f,
    -kPi / 2.0f,
    -kPi / 6.0f,
    0.0f,
    kPi / 6.0f,
    kPi / 2.0f,
    2.0f * kPi / 3.0f,
    kPi,
};

constexpr Vector2 kVectors[] = {
    {1.0f, 0.0f},
    {0.0f, 1.0f},
    {-1.0f, 0.5f},
    {0.25f, -0.75f},
};
}  // namespace

TEST(ParkTransform, ParkThenInvParkReturnsOriginalAlphaBeta) {
  for (const auto theta : kAngles) {
    for (const auto input : kVectors) {
      float d = 0.0f;
      float q = 0.0f;
      float alpha = 0.0f;
      float beta = 0.0f;

      park(theta, input.a, input.b, d, q);
      invPark(theta, d, q, alpha, beta);

      DOUBLES_EQUAL(input.a, alpha, kTolerance);
      DOUBLES_EQUAL(input.b, beta, kTolerance);
    }
  }
}

TEST(ParkTransform, InvParkThenParkReturnsOriginalDq) {
  for (const auto theta : kAngles) {
    for (const auto input : kVectors) {
      float alpha = 0.0f;
      float beta = 0.0f;
      float d = 0.0f;
      float q = 0.0f;

      invPark(theta, input.a, input.b, alpha, beta);
      park(theta, alpha, beta, d, q);

      DOUBLES_EQUAL(input.a, d, kTolerance);
      DOUBLES_EQUAL(input.b, q, kTolerance);
    }
  }
}
