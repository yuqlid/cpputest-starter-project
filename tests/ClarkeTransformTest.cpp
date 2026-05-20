#include "CppUTest/TestHarness.h"

#include <cmath>

#include "clarke.hpp"

TEST_GROUP(ClarkeTransform) {};

namespace {
constexpr float kTolerance = 0.000001f;
constexpr float kSqrt3By2 = 0.86602540378443864676f;

struct Uvw {
  float u;
  float v;
  float w;
};

struct AlphaBeta {
  float alpha;
  float beta;
};

Uvw relativeUvwFromAlphaBeta(const float alpha, const float beta) {
  return {
      alpha,
      -0.5f * alpha + kSqrt3By2 * beta,
      -0.5f * alpha - kSqrt3By2 * beta,
  };
}

Uvw removeCommonMode(const Uvw input) {
  const float common_mode = (input.u + input.v + input.w) / 3.0f;
  return {
      input.u - common_mode,
      input.v - common_mode,
      input.w - common_mode,
  };
}

void checkClarke(const Uvw input, const AlphaBeta expected) {
  float alpha = 0.0f;
  float beta = 0.0f;

  clarke(input.u, input.v, input.w, alpha, beta);

  DOUBLES_EQUAL(expected.alpha, alpha, kTolerance);
  DOUBLES_EQUAL(expected.beta, beta, kTolerance);
}
}  // namespace

TEST(ClarkeTransform, MapsRelativePhaseAxesToAlphaBeta) {
  checkClarke({1.0f, -0.5f, -0.5f}, {1.0f, 0.0f});
  checkClarke({-0.5f, 1.0f, -0.5f}, {-0.5f, kSqrt3By2});
  checkClarke({-0.5f, -0.5f, 1.0f}, {-0.5f, -kSqrt3By2});
}

TEST(ClarkeTransform, RecoversAlphaBetaFromRelativeInverseFormula) {
  const AlphaBeta vectors[] = {
      {0.0f, 0.0f},
      {1.0f, 0.0f},
      {0.0f, 1.0f},
      {-0.25f, 0.75f},
      {0.6f, -0.4f},
  };

  for (const auto vector : vectors) {
    checkClarke(relativeUvwFromAlphaBeta(vector.alpha, vector.beta), vector);
  }
}

TEST(ClarkeTransform, CommonModeInputMustBePreprocessedBeforeUse) {
  const AlphaBeta expected = {0.3f, -0.7f};
  const Uvw relative = relativeUvwFromAlphaBeta(expected.alpha, expected.beta);
  const Uvw adc_like_input = {
      relative.u + 1.65f,
      relative.v + 1.65f,
      relative.w + 1.65f,
  };

  checkClarke(removeCommonMode(adc_like_input), expected);
}
