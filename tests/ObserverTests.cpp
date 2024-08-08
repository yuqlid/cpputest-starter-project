#include <cmath>
#include <fstream>
#include <iostream>

#include "CppUTest/TestHarness.h"
#include "observer.hpp"

extern "C" {
/*
 * Add your c-only include files here
 */
}

#define _USE_MATH_DEFINES  // for C++

static constexpr float pi = 3.14159265358979323846264338327950288;
static constexpr float fs_hz = 20000.0f;
static constexpr float bandwitdh = 300.0f;

/**
 * @brief -pi ~ +piにまるめる
 *
 * @tparam T
 * @param input
 * @return T
 */
template <typename T>
inline T wrapAngle(T input) {
  constexpr T two_pi = 2.0f * pi;
  T temp = input;
  while (temp < -pi) {
    temp += two_pi;
  }
  while (temp > pi) {
    temp -= two_pi;
  }
  return temp;
}

TEST_GROUP(Observer){void setup(){}

                     void teardown(){}};

TEST(Observer, input_step) {
  constexpr uint16_t period = 1024;
  std::array<float, period> theta_meas;
  motimoro_observer::Observer<float> Observer1(bandwitdh, 1.0f / fs_hz);
  odrive_observer::Observer Observer2(bandwitdh, 1 / fs_hz);
  constexpr float vel_npm = 3000.0f;
  constexpr float vel = vel_npm * pi / 60.0f;  // rad/s

  std::ofstream outfile("observer_response_step.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    float theta_temp = pi / 2;
    if (i == 0) theta_temp = 0.0f;
    theta_meas[i] = wrapAngle<float>(theta_temp);
  }

  outfile << "tims,theta_in,theta_morimoto,theta_tm,theta_odrive,vel_in,vel_"
             "morimoto,vel_tm,vel_odrive"
          << std::endl;

  for (size_t i = 0; i < period; ++i) {
    Observer1.process(theta_meas[i]);
    Observer2.process(theta_meas[i]);
    outfile << static_cast<float>(i) / fs_hz << ", " << theta_meas[i] << ", "
            << Observer1.getPos() << ", " << Observer2.getPos() << ", " << vel
            << ", " << Observer1.getVel() << ", " << Observer2.getVel()
            << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored observer_response_step.csv" << std::endl;
}

TEST(Observer, input_sinwave) {
  constexpr uint16_t period = 1024;
  std::array<float, period> theta_meas;
  motimoro_observer::Observer<float> Observer1(bandwitdh, 1.0f / fs_hz);
  odrive_observer::Observer Observer2(bandwitdh, 1 / fs_hz);
  constexpr float vel_npm = 1000.0f;
  float vel = vel_npm * pi / 60.0f;  // rad/s

  std::ofstream outfile("observer_response_sin.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  constexpr float two_pi = 2.0f * pi;
  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    theta_meas[i] = wrapAngle<float>(std::sin(two_pi * i / period));
  }

  outfile << "tims,theta_in,theta_morimoto,theta_tm,theta_odrive,vel_in,vel_"
             "morimoto,vel_tm,vel_odrive"
          << std::endl;

  for (size_t i = 0; i < period; ++i) {
    Observer1.process(theta_meas[i]);
    Observer2.process(theta_meas[i]);
    outfile << static_cast<float>(i) / fs_hz << ", " << theta_meas[i] << ", "
            << Observer1.getPos() << ", " << Observer2.getPos() << ", " << vel
            << ", " << Observer1.getVel() << ", " << Observer2.getVel()
            << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored observer_response_sin.csv" << std::endl;
}

TEST(Observer, input_ramp) {
  constexpr uint16_t period = 1024;
  std::array<float, period> theta_meas;
  motimoro_observer::Observer<float> Observer1(bandwitdh, 1.0f / fs_hz);
  odrive_observer::Observer Observer2(bandwitdh, 1 / fs_hz);
  constexpr float vel_npm = 3000.0f;
  constexpr float vel = vel_npm * pi / 60.0f;  // rad/s

  std::ofstream outfile("observer_response_ramp.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    float theta_temp = i * vel / fs_hz + 0.0f;
    theta_meas[i] = wrapAngle<float>(theta_temp);
  }

  outfile << "tims,theta_in,theta_morimoto,theta_tm,theta_odrive,vel_in,vel_"
             "morimoto,vel_tm,vel_odrive"
          << std::endl;

  for (size_t i = 0; i < period; ++i) {
    Observer1.process(theta_meas[i]);
    Observer2.process(theta_meas[i]);
    outfile << static_cast<float>(i) / fs_hz << ", " << theta_meas[i] << ", "
            << Observer1.getPos() << ", " << Observer2.getPos() << ", " << vel
            << ", " << Observer1.getVel() << ", " << Observer2.getVel()
            << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored observer_response_ramp.csv" << std::endl;
}