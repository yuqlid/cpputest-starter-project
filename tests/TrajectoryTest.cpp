#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <type_traits>
#include <vector>

#include "CppUTest/TestHarness.h"
#include "trajectory.hpp"

extern "C" {
/*
 * Add your c-only include files here
 */
}

#define _USE_MATH_DEFINES  // for C++

TEST_GROUP(Trajectory){void setup(){}

                       void teardown(){}};

constexpr float startPos = 0.0f * 3.14 / 3.0f;
constexpr float endPos = 2.0f * 3.14 / 3.0f;
constexpr float maxVel = 10.0f;
constexpr float maxAccel = 200.0f;
constexpr float timeStep = 1.0f / 20000.0f;

TEST(Trajectory, test1) {
  TrapezoidalTrajectory<float> traj(startPos, endPos, maxVel, maxAccel,
                                    timeStep);

  std::ofstream outfile("trajectory.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  outfile << "time, pos,vel" << std::endl;
  while (!traj.isFinished()) {
    float pos = traj.nextStep();
    outfile << traj.getCurrentTime() << "," << pos << ","
            << traj.getCurrentVel() << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored trajectory.csv" << std::endl;
}
