#include "CppUTest/TestHarness.h"

#include <cstdint>

#include "adc_manager.hpp"
#include "control_manager.hpp"
#include "current_measurement.hpp"
#include "fault_log.hpp"
#include "fault_manager.hpp"
#include "pwm_manager.hpp"
#include "voltage_manager.hpp"

namespace {

int zero_duty_count = 0;
int pwm_idle_count = 0;
uint32_t tick = 1234;
ControlState control_state = ControlState::Current();
PwmState pwm_state = PwmState::PwmActive();
uvw_t<float> phase_current{1.0f, 2.0f, 3.0f};

void resetFakes() {
  zero_duty_count = 0;
  pwm_idle_count = 0;
  tick = 1234;
  control_state = ControlState::Current();
  pwm_state = PwmState::PwmActive();
  phase_current = {1.0f, 2.0f, 3.0f};
}

}  // namespace

extern "C" uint32_t HAL_GetTick(void) { return tick; }

ControlState getControlState() { return control_state; }

void setPwmZeroDuty() { ++zero_duty_count; }

void setPwmIdle() {
  ++pwm_idle_count;
  pwm_state = PwmState::PwmIdle();
}

const PwmState& getPwmTimerState() { return pwm_state; }

float getBusVoltage() { return 24.0f; }

float getMcuVoltage() { return 3.3f; }

uint16_t getVrefVal() { return 1500; }

uint16_t getTempVal() { return 1600; }

uint16_t getVbatVal() { return 1700; }

const uvw_t<float>& getPhaseCurrent() { return phase_current; }

TEST_GROUP(FaultManager) {
  void setup() {
    resetFakes();
    getFaultLog().notifyRecovered();
    getFaultLog().clearHistory();
  }
};

TEST(FaultManager, TriggerFaultStopsPwmAndStoresFault) {
  triggerFault(ErrorCode::OverCurrent());

  LONGS_EQUAL(1, zero_duty_count);
  LONGS_EQUAL(1, pwm_idle_count);
  CHECK_TRUE(hasActiveFault());
  UNSIGNED_LONGS_EQUAL(1, getFaultLog().size());
  LONGS_EQUAL(static_cast<long>(ErrorCode::Code::kOverCurrent),
              static_cast<long>(getFaultLog().latest().error_code));
  LONGS_EQUAL(static_cast<long>(PwmState::Code::kPwmIdle),
              static_cast<long>(getFaultLog().latest().pwm_state));
}

TEST(FaultManager, TriggerNoErrorDoesNothing) {
  triggerFault(ErrorCode::NoError());

  LONGS_EQUAL(0, zero_duty_count);
  LONGS_EQUAL(0, pwm_idle_count);
  CHECK_FALSE(hasActiveFault());
  CHECK_TRUE(getFaultLog().empty());
}

TEST(FaultManager, NotifyRecoveredClearsFaultButPreservesHistory) {
  triggerFault(ErrorCode::OverCurrent());

  notifyFaultRecovered();

  CHECK_FALSE(hasActiveFault());
  UNSIGNED_LONGS_EQUAL(1, getFaultLog().size());
}

TEST(FaultManager, ClearHistoryPreservesActiveFault) {
  triggerFault(ErrorCode::OverCurrent());

  getFaultLog().clearHistory();

  CHECK_TRUE(hasActiveFault());
  CHECK_TRUE(getFaultLog().empty());
}

TEST(FaultManager, StoreNoErrorDoesNotRecoverActiveFault) {
  triggerFault(ErrorCode::OverCurrent());

  getFaultLog().store(ErrorCode::NoError());

  CHECK_TRUE(hasActiveFault());
  UNSIGNED_LONGS_EQUAL(1, getFaultLog().size());
}

TEST(FaultManager, DuplicateActiveFaultIsNotStoredTwice) {
  triggerFault(ErrorCode::OverCurrent());
  triggerFault(ErrorCode::OverCurrent());

  UNSIGNED_LONGS_EQUAL(1, getFaultLog().size());
}
