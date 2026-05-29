#include "CppUTest/TestHarness.h"

#include <cstdint>

#include "ds402_state_machine.hpp"

namespace {

constexpr uint16_t kDisableVoltageControlWord = 0b00000000;
constexpr uint16_t kQuickStopControlWord = 0b00000010;
constexpr uint16_t kShutdownControlWord = 0b00000110;
constexpr uint16_t kSwitchOnControlWord = 0b00000111;
constexpr uint16_t kEnableOperationControlWord = 0b00001111;
constexpr uint16_t kFaultResetControlWord = 0b10000000;

class FakeDS402Drive final : public DS402DriveInterface {
 public:
  int initialize_count = 0;
  int enable_power_count = 0;
  int disable_power_count = 0;
  int enable_drive_count = 0;
  int disable_drive_count = 0;
  int quick_stop_count = 0;
  int handle_fault_count = 0;
  int state_action_count = 0;
  DS402State last_action_state = DS402State::NotReadyToSwitchOn();

  DS402TransitionStatus initialize_status = DS402TransitionStatus::kFinished;
  DS402TransitionStatus enable_power_status = DS402TransitionStatus::kFinished;
  DS402TransitionStatus disable_power_status = DS402TransitionStatus::kFinished;
  DS402TransitionStatus enable_drive_status = DS402TransitionStatus::kFinished;
  DS402TransitionStatus quick_stop_status = DS402TransitionStatus::kFinished;
  DS402TransitionStatus disable_drive_status = DS402TransitionStatus::kFinished;
  DS402TransitionStatus handle_fault_status = DS402TransitionStatus::kFinished;
  DS402TransitionStatus state_action_status = DS402TransitionStatus::kFinished;

  DS402TransitionStatus initialize() override {
    ++initialize_count;
    return initialize_status;
  }

  DS402TransitionStatus enablePower() override {
    ++enable_power_count;
    return enable_power_status;
  }

  DS402TransitionStatus disablePower() override {
    ++disable_power_count;
    return disable_power_status;
  }

  DS402TransitionStatus enableDrive() override {
    ++enable_drive_count;
    return enable_drive_status;
  }

  DS402TransitionStatus disableDrive() override {
    ++disable_drive_count;
    return disable_drive_status;
  }

  DS402TransitionStatus activateQuickStop() override {
    ++quick_stop_count;
    return quick_stop_status;
  }

  DS402TransitionStatus handleFault() override {
    ++handle_fault_count;
    return handle_fault_status;
  }

  DS402TransitionStatus doStateAction(DS402State current_state) override {
    ++state_action_count;
    last_action_state = current_state;
    return state_action_status;
  }
};

}  // namespace

TEST_GROUP(DS402StateMachine) {
  FakeDS402Drive drive;
  DriveStateMachine machine{drive};
};

TEST(DS402StateMachine, InitialUpdateInitializesDriveAndMovesToSwitchOnDisabled) {
  CHECK_TRUE(machine.getState() == DS402State::NotReadyToSwitchOn());

  const DS402State state = machine.updateState();

  CHECK_TRUE(state == DS402State::SwitchOnDisabled());
  LONGS_EQUAL(1, drive.initialize_count);
  LONGS_EQUAL(1, drive.state_action_count);
  CHECK_TRUE(drive.last_action_state == DS402State::SwitchOnDisabled());
}

TEST(DS402StateMachine, NormalControlwordSequenceReachesOperationEnabled) {
  machine.updateState();

  machine.setControlWord(kShutdownControlWord);
  CHECK_TRUE(machine.updateState() == DS402State::ReadyToSwitchOn());

  machine.setControlWord(kSwitchOnControlWord);
  CHECK_TRUE(machine.updateState() == DS402State::SwitchedOn());
  LONGS_EQUAL(1, drive.enable_power_count);

  machine.setControlWord(kEnableOperationControlWord);
  CHECK_TRUE(machine.updateState() == DS402State::OperationEnabled());
  LONGS_EQUAL(1, drive.enable_drive_count);

  const uint16_t expected_status_word =
      static_cast<uint16_t>(DS402State::OperationEnabled()) |
      kStatusWordVoltageEnabledMask;
  LONGS_EQUAL(expected_status_word, machine.getStatusWord());
}

TEST(DS402StateMachine, OngoingDriveTransitionHoldsCurrentState) {
  machine.updateState();
  machine.setControlWord(kShutdownControlWord);
  machine.updateState();

  drive.enable_power_status = DS402TransitionStatus::kOngoing;
  machine.setControlWord(kSwitchOnControlWord);

  CHECK_TRUE(machine.updateState() == DS402State::ReadyToSwitchOn());
  LONGS_EQUAL(1, drive.enable_power_count);
}

TEST(DS402StateMachine, DriveFaultMovesThroughFaultReactionToFault) {
  machine.updateState();
  machine.setControlWord(kShutdownControlWord);
  machine.updateState();

  drive.enable_power_status = DS402TransitionStatus::kFault;
  machine.setControlWord(kSwitchOnControlWord);

  CHECK_TRUE(machine.updateState() == DS402State::FaultReactionActive());

  drive.enable_power_status = DS402TransitionStatus::kFinished;
  CHECK_TRUE(machine.updateState() == DS402State::Fault());
  LONGS_EQUAL(1, drive.handle_fault_count);
}

TEST(DS402StateMachine, FaultResetRequiresRisingEdgeOfControlwordBit7) {
  machine.updateState();
  machine.Fault();
  CHECK_TRUE(machine.updateState() == DS402State::Fault());

  machine.setControlWord(kFaultResetControlWord);
  CHECK_TRUE(machine.updateState() == DS402State::SwitchOnDisabled());

  machine.Fault();
  CHECK_TRUE(machine.updateState() == DS402State::Fault());

  machine.setControlWord(kFaultResetControlWord);
  CHECK_TRUE(machine.updateState() == DS402State::Fault());

  machine.setControlWord(kDisableVoltageControlWord);
  machine.updateState();
  machine.setControlWord(kFaultResetControlWord);
  CHECK_TRUE(machine.updateState() == DS402State::SwitchOnDisabled());
}

TEST(DS402StateMachine, QuickStopFromOperationEnabledActivatesQuickStop) {
  machine.updateState();
  machine.setControlWord(kShutdownControlWord);
  machine.updateState();
  machine.setControlWord(kSwitchOnControlWord);
  machine.updateState();
  machine.setControlWord(kEnableOperationControlWord);
  machine.updateState();

  machine.setControlWord(kQuickStopControlWord);

  CHECK_TRUE(machine.updateState() == DS402State::QuickStopActive());
  LONGS_EQUAL(1, drive.quick_stop_count);
}
