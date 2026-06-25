#include "CppUTest/TestHarness.h"
#include "cia402_manager.hpp"

TEST_GROUP(Cia402Manager){};

TEST(Cia402Manager, ShouldNotRequestDs402FaultWhenNoActiveFault) {
  CHECK_FALSE(
      shouldRequestDs402Fault(ds402::State::OperationEnabled(), false));
}

TEST(Cia402Manager, ShouldRequestDs402FaultFromNormalStates) {
  CHECK_TRUE(
      shouldRequestDs402Fault(ds402::State::SwitchOnDisabled(), true));
  CHECK_TRUE(shouldRequestDs402Fault(ds402::State::ReadyToSwitchOn(), true));
  CHECK_TRUE(shouldRequestDs402Fault(ds402::State::SwitchedOn(), true));
  CHECK_TRUE(shouldRequestDs402Fault(ds402::State::OperationEnabled(), true));
  CHECK_TRUE(shouldRequestDs402Fault(ds402::State::QuickStopActive(), true));
}

TEST(Cia402Manager, ShouldNotRequestDs402FaultFromFaultStates) {
  CHECK_FALSE(
      shouldRequestDs402Fault(ds402::State::FaultReactionActive(), true));
  CHECK_FALSE(shouldRequestDs402Fault(ds402::State::Fault(), true));
}
