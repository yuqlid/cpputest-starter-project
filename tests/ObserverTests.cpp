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

TEST_GROUP(Observer){void setup(){}

                     void teardown(){}};

TEST(Observer, test1) {
  constexpr float pi = 3.14159265358979323846264338327950288;
  constexpr float two_pi = 2.0f * pi;
  constexpr float fs_hz = 20000.0f;
  constexpr uint16_t period = 1024;
  std::array<float, period> theta_meas;
  Observer Observer(300.0f, 1 / fs_hz);
  Observer.updateBandWitdh(100.0f, 1.0f);
  constexpr float vel_npm = 1000.0f;
  constexpr float vel = vel_npm * pi / 60.0f;  // rad/s

  std::ofstream outfile("observer_test.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    float theta_temp = i * vel / fs_hz + 0.0f;
    while (theta_temp < -pi) {
      theta_temp += two_pi;
    }
    while (theta_temp > pi) {
      theta_temp -= two_pi;
    }
    theta_meas[i] = theta_temp;
  }

  outfile << "tims,theta_in,theta_est,vel_in,vel_est" << std::endl;

  for (size_t i = 0; i < period; ++i) {
    Observer.process(theta_meas[i]);
    outfile << static_cast<float>(i) / fs_hz << ", " << theta_meas[i] << ", "
            << Observer.getPos() << ", " << vel << ", " << Observer.getVel()
            << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored observer_test.csv" << std::endl;
}

TEST(Observer, test2) {
  constexpr float pi = 3.14159265358979323846264338327950288;
  constexpr float two_pi = 2.0f * pi;
  constexpr float fs_hz = 20000.0f;
  constexpr uint16_t period = 1024;
  std::array<float, period> theta_meas;
  constexpr float vel_npm = 1000.0f;
  constexpr float vel = vel_npm * pi / 60.0f;  // rad/s
  Observer Observer(300.0f, 1 / fs_hz);
  Observer.updateBandWitdh(300.0f, 1.3f);

  std::ofstream outfile("observer_test2.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    float theta_temp = i * vel / fs_hz + 0.0f;
    while (theta_temp < 0.0f) {
      theta_temp += two_pi;
    }
    while (theta_temp > two_pi) {
      theta_temp -= two_pi;
    }

    float temp2 = theta_temp / (two_pi / 6.0f);
    temp2 *= two_pi / 6.0f;
    while (temp2 < -pi) {
      temp2 += two_pi;
    }
    while (temp2 > pi) {
      temp2 -= two_pi;
    }
    theta_meas[i] = temp2;
  }

  outfile << "tims,theta_in,theta_est,vel_in,vel_est" << std::endl;

  for (size_t i = 0; i < period; ++i) {
    Observer.process(theta_meas[i]);
    outfile << static_cast<float>(i) / fs_hz << ", " << theta_meas[i] << ", "
            << Observer.getPos() << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored observer_test2.csv" << std::endl;
}

TEST(Observer, test3) {
  constexpr float pi = 3.14159265358979323846264338327950288;
  constexpr float two_pi = 2.0f * pi;
  constexpr float fs_hz = 20000.0f;
  constexpr uint16_t period = 1024;
  std::array<float, period> theta_meas;
  Observer Observer(300.0f, 1 / fs_hz);
  Observer.updateBandWitdh(100.0f, 1.0f);
  constexpr float vel_npm = 1000.0f;
  float vel = vel_npm * pi / 60.0f;  // rad/s

  std::ofstream outfile("observer_test3.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    float theta_temp = pi * std::sin(two_pi * i / period);
    while (theta_temp < -pi) {
      theta_temp += two_pi;
    }
    while (theta_temp > pi) {
      theta_temp -= two_pi;
    }
    theta_meas[i] = theta_temp;
  }

  outfile << "tims,theta_in,theta_est,vel_in,vel_est" << std::endl;

  for (size_t i = 0; i < period; ++i) {
    vel = pi * std::cos(two_pi * i / period);
    Observer.process(theta_meas[i]);
    outfile << static_cast<float>(i) / fs_hz << ", " << theta_meas[i] << ", "
            << Observer.getPos() << ", " << vel << ", " << Observer.getVel()
            << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored observer_test3.csv" << std::endl;
}


TEST(Observer, test4) {
  constexpr float pi = 3.14159265358979323846264338327950288;
  constexpr float two_pi = 2.0f * pi;
  constexpr float fs_hz = 20000.0f;
  constexpr uint16_t period = 1024;
  std::array<float, period> theta_meas;
  Observer Observer(300.0f, 1 / fs_hz);
  Observer.updateBandWitdh(100.0f, 1.0f);
  constexpr float vel_npm = 1000.0f;
  constexpr float vel = vel_npm * pi / 60.0f;  // rad/s

  std::ofstream outfile("observer_test4.csv");
  // ファイルが正しく開けたか確認する
  if (!outfile) {
    FAIL("file open failed");
  }

  // 入力波形の時系列データ作成
  for (int i = 0; i < period; ++i) {
    float theta_temp = i * vel / fs_hz + 0.0f;
    while (theta_temp < -pi) {
      theta_temp += two_pi;
    }
    while (theta_temp > pi) {
      theta_temp -= two_pi;
    }
    theta_meas[i] = theta_temp;
  }

  outfile << "tims,theta_in,theta_est,vel_in,vel_est" << std::endl;

  for (size_t i = 0; i < period; ++i) {
    Observer.process2(theta_meas[i]);
    outfile << static_cast<float>(i) / fs_hz << ", " << theta_meas[i] << ", "
            << Observer.getPos() << ", " << vel << ", " << Observer.getVel()
            << std::endl;
  }
  // ファイルを閉じる
  outfile.close();

  std::cout << "stored observer_test4.csv" << std::endl;
}