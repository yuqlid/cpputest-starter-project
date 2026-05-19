#include <cmath>
#include <fstream>
#include <string>

#include "CppUTest/TestHarness.h"
#include "TestOutputPath.hpp"
#include "trajectory.hpp"

TEST_GROUP(Trajectory) {
  const char *csv_file_name = nullptr;

  void teardown() {
    if (csv_file_name != nullptr) {
      TestOutputPath::copyToLatest("Trajectory", csv_file_name);
    }
  }
};

namespace {
constexpr double kTolerance = 1.0e-5;

template <typename T>
void advanceToEnd(TrapezoidalTrajectory<T> &trajectory) {
  int guard = 0;
  while (!trajectory.isFinished() && guard < 10000) {
    trajectory.nextStep();
    ++guard;
  }
  CHECK(guard < 10000);
}

void writeTrajectoryCsv(const std::string &file_name,
                        TrapezoidalTrajectory<double> trajectory) {
  const auto path = TestOutputPath::makeCsvPath("Trajectory", file_name);
  std::ofstream csv(path);

  CHECK_TEXT(csv.is_open(), "trajectory csv open failed");

  csv << "time,pos,vel,accel,progress\n";

  int guard = 0;
  while (!trajectory.isFinished() && guard < 10000) {
    const double sample_time = trajectory.getCurrentTime();
    const auto state = trajectory.nextStep();
    csv << sample_time << "," << state.pos << "," << state.vel << ","
        << state.accel << "," << trajectory.getProgress() << "\n";
    ++guard;
  }

  CHECK(guard < 10000);

  const auto final_state = trajectory.nextStep();
  csv << trajectory.getCurrentTime() << "," << final_state.pos << ","
      << final_state.vel << "," << final_state.accel << ","
      << trajectory.getProgress() << "\n";

  // writeTrajectoryCsvを抜ける際にcsvのデストラクタが呼び出されるため自動的に内部でclose()が実行される
}
}  // namespace

TEST(Trajectory, TrapezoidalMoveReachesEndPosition) {
  TrapezoidalTrajectory<double> trajectory(0.0, 10.0, 2.0, 1.0, 0.1);

  DOUBLES_EQUAL(7.0, trajectory.getTotalTime(), kTolerance);

  advanceToEnd(trajectory);
  const auto state = trajectory.nextStep();

  CHECK(trajectory.isFinished());
  DOUBLES_EQUAL(10.0, state.pos, kTolerance);
  DOUBLES_EQUAL(0.0, state.vel, kTolerance);
  DOUBLES_EQUAL(0.0, state.accel, kTolerance);
}

TEST(Trajectory, TriangularMoveUsesReachablePeakVelocity) {
  TrapezoidalTrajectory<double> trajectory(0.0, 1.0, 10.0, 2.0, 0.1);

  DOUBLES_EQUAL(std::sqrt(2.0), trajectory.getTotalTime(), kTolerance);

  advanceToEnd(trajectory);
  const auto state = trajectory.nextStep();

  DOUBLES_EQUAL(1.0, state.pos, kTolerance);
  DOUBLES_EQUAL(0.0, state.vel, kTolerance);
}

TEST(Trajectory, ReverseMoveKeepsVelocitySignNegative) {
  TrapezoidalTrajectory<double> trajectory(5.0, 1.0, 2.0, 4.0, 0.1);

  const auto first = trajectory.nextStep();
  const auto second = trajectory.nextStep();

  DOUBLES_EQUAL(5.0, first.pos, kTolerance);
  CHECK(second.vel < decltype(second.vel)(0));

  advanceToEnd(trajectory);
  const auto state = trajectory.nextStep();

  DOUBLES_EQUAL(1.0, state.pos, kTolerance);
  DOUBLES_EQUAL(0.0, state.vel, kTolerance);
}

TEST(Trajectory, InvalidLimitsMakeStationaryTrajectory) {
  TrapezoidalTrajectory<double> trajectory(2.0, 4.0, 0.0, 1.0, 0.1);

  CHECK(trajectory.isFinished());
  DOUBLES_EQUAL(0.0, trajectory.getTotalTime(), kTolerance);
  DOUBLES_EQUAL(1.0, trajectory.getProgress(), kTolerance);

  const auto state = trajectory.nextStep();
  DOUBLES_EQUAL(2.0, state.pos, kTolerance);
  DOUBLES_EQUAL(0.0, state.vel, kTolerance);
}

TEST(Trajectory, TimeAdvancesByOneStepPerSample) {
  TrapezoidalTrajectory<double> trajectory(0.0, 10.0, 2.0, 1.0, 0.1);

  trajectory.nextStep();
  DOUBLES_EQUAL(0.1, trajectory.getCurrentTime(), kTolerance);

  trajectory.nextStep();
  DOUBLES_EQUAL(0.2, trajectory.getCurrentTime(), kTolerance);
}

TEST(Trajectory, GenerateCsvForVisualInspection_trap) {
  csv_file_name = "trapezoidal.csv";
  writeTrajectoryCsv(csv_file_name,
                     TrapezoidalTrajectory<double>(0.0, 10.0, 2.0, 1.0, 0.01));
}

TEST(Trajectory, GenerateCsvForVisualInspection_triangle) {
  csv_file_name = "triangular.csv";
  writeTrajectoryCsv(csv_file_name,
                     TrapezoidalTrajectory<double>(0.0, 1.0, 10.0, 2.0, 0.01));
}

TEST(Trajectory, GenerateCsvForVisualInspection_revese) {
  csv_file_name = "reverse.csv";
  writeTrajectoryCsv(csv_file_name,
                     TrapezoidalTrajectory<double>(5.0, 1.0, 2.0, 4.0, 0.01));
}