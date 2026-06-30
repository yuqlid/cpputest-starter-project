#include "CppUTest/TestHarness.h"

#include "ds402_drive.hpp"
#include "fault_log.hpp"
#include "fault_manager.hpp"

namespace {

bool power_supply_ready = false;

void clearFaultState() {
  getFaultLog().notifyRecovered();
  getFaultLog().clearHistory();
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
