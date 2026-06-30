#include "CppUTest/TestHarness.h"

#include "cia402_manager.hpp"
#include "ds402_drive.hpp"
#include "fault_log.hpp"
#include "fault_manager.hpp"

namespace {

bool power_supply_ready = false;
constexpr uint16_t kFaultResetControlWord = 0b10000000;

void clearFaultState() {
  getFaultLog().notifyRecovered();
  getFaultLog().clearHistory();
}

void syncActiveFaultToDs402(ds402::StateMachine& machine) {
  if (shouldRequestDs402Fault(machine.getState(), hasActiveFault())) {
    machine.requestFault();
  }
}

void moveActiveFaultToDs402Fault(ds402::StateMachine& machine) {
  syncActiveFaultToDs402(machine);
  CHECK_TRUE(machine.getState() == ds402::State::FaultReactionActive());
  CHECK_TRUE(machine.update() == ds402::State::Fault());
}

}  // namespace

bool isPowerSupplyReady() { return power_supply_ready; }

void setPwmActive() {}

TEST_GROUP(AppDS402Drive) {
  void setup() {
    power_supply_ready = false;
    clearFaultState();
  }
};

TEST(AppDS402Drive, InitializeRecoversVoltageFaultWhenPowerSupplyIsReady) {
  AppDS402Drive drive;
  triggerFault(ErrorCode::DCLinkUnderVoltage());
  power_supply_ready = true;

  CHECK_TRUE(drive.initialize() == ds402::TransitionStatus::kFinished);
  CHECK_FALSE(hasActiveFault());
  LONGS_EQUAL(static_cast<long>(ErrorCode::Code::kNoError),
              static_cast<long>(
                  static_cast<ErrorCode::Code>(getActiveFaultCode())));
}

TEST(AppDS402Drive, InitializeHoldsVoltageFaultWhenPowerSupplyIsNotReady) {
  AppDS402Drive drive;
  triggerFault(ErrorCode::DCLinkUnderVoltage());

  CHECK_TRUE(drive.initialize() == ds402::TransitionStatus::kOngoing);
  CHECK_TRUE(hasActiveFault());
  LONGS_EQUAL(static_cast<long>(ErrorCode::Code::kDCLinkUnderVoltage),
              static_cast<long>(
                  static_cast<ErrorCode::Code>(getActiveFaultCode())));
}

TEST(AppDS402Drive, InitializeHoldsUnsupportedFault) {
  AppDS402Drive drive;
  triggerFault(ErrorCode::OverCurrent());
  power_supply_ready = true;

  CHECK_TRUE(drive.initialize() == ds402::TransitionStatus::kOngoing);
  CHECK_TRUE(hasActiveFault());
  LONGS_EQUAL(static_cast<long>(ErrorCode::Code::kOverCurrent),
              static_cast<long>(
                  static_cast<ErrorCode::Code>(getActiveFaultCode())));
}

TEST(AppDS402Drive,
     FaultResetReturnsToSwitchOnDisabledWhenVoltageFaultRecovered) {
  AppDS402Drive drive;
  ds402::StateMachine machine{drive};
  power_supply_ready = true;
  CHECK_TRUE(machine.update() == ds402::State::SwitchOnDisabled());

  triggerFault(ErrorCode::DCLinkUnderVoltage());
  moveActiveFaultToDs402Fault(machine);

  machine.setControlWord(kFaultResetControlWord);

  CHECK_TRUE(machine.update() == ds402::State::SwitchOnDisabled());
  CHECK_FALSE(hasActiveFault());
}

TEST(AppDS402Drive, FaultResetHoldsFaultWhenVoltageFaultIsNotRecovered) {
  AppDS402Drive drive;
  ds402::StateMachine machine{drive};
  power_supply_ready = true;
  CHECK_TRUE(machine.update() == ds402::State::SwitchOnDisabled());

  triggerFault(ErrorCode::DCLinkUnderVoltage());
  power_supply_ready = false;
  moveActiveFaultToDs402Fault(machine);

  machine.setControlWord(kFaultResetControlWord);

  CHECK_TRUE(machine.update() == ds402::State::Fault());
  CHECK_TRUE(hasActiveFault());
}

TEST(AppDS402Drive, FaultResetHoldsFaultForUnsupportedFault) {
  AppDS402Drive drive;
  ds402::StateMachine machine{drive};
  power_supply_ready = true;
  CHECK_TRUE(machine.update() == ds402::State::SwitchOnDisabled());

  triggerFault(ErrorCode::OverCurrent());
  moveActiveFaultToDs402Fault(machine);

  machine.setControlWord(kFaultResetControlWord);

  CHECK_TRUE(machine.update() == ds402::State::Fault());
  CHECK_TRUE(hasActiveFault());
}
