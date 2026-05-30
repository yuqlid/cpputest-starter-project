#include <cstdint>

#include "CppUTest/TestHarness.h"
#include "cia402_homing.hpp"
#include "ds402_state_machine.hpp"

namespace {

constexpr uint16_t kDisableVoltageControlWord = 0b00000000;
constexpr uint16_t kQuickStopControlWord = 0b00000010;
constexpr uint16_t kShutdownControlWord = 0b00000110;
constexpr uint16_t kSwitchOnControlWord = 0b00000111;
constexpr uint16_t kEnableOperationControlWord = 0b00001111;
constexpr uint16_t kFaultResetControlWord = 0b10000000;

class FakeDS402Drive final : public ds402::DriveInterface {
 public:
  int initialize_count = 0;
  int enable_power_count = 0;
  int disable_power_count = 0;
  int enable_drive_count = 0;
  int disable_drive_count = 0;
  int quick_stop_count = 0;
  int handle_fault_count = 0;
  int state_action_count = 0;
  ds402::State last_action_state = ds402::State::NotReadyToSwitchOn();

  ds402::TransitionStatus initialize_status =
      ds402::TransitionStatus::kFinished;
  ds402::TransitionStatus enable_power_status =
      ds402::TransitionStatus::kFinished;
  ds402::TransitionStatus disable_power_status =
      ds402::TransitionStatus::kFinished;
  ds402::TransitionStatus enable_drive_status =
      ds402::TransitionStatus::kFinished;
  ds402::TransitionStatus quick_stop_status =
      ds402::TransitionStatus::kFinished;
  ds402::TransitionStatus disable_drive_status =
      ds402::TransitionStatus::kFinished;
  ds402::TransitionStatus handle_fault_status =
      ds402::TransitionStatus::kFinished;
  ds402::TransitionStatus state_action_status =
      ds402::TransitionStatus::kFinished;

  ds402::TransitionStatus initialize() override {
    ++initialize_count;
    return initialize_status;
  }

  ds402::TransitionStatus enablePower() override {
    ++enable_power_count;
    return enable_power_status;
  }

  ds402::TransitionStatus disablePower() override {
    ++disable_power_count;
    return disable_power_status;
  }

  ds402::TransitionStatus enableDrive() override {
    ++enable_drive_count;
    return enable_drive_status;
  }

  ds402::TransitionStatus disableDrive() override {
    ++disable_drive_count;
    return disable_drive_status;
  }

  ds402::TransitionStatus activateQuickStop() override {
    ++quick_stop_count;
    return quick_stop_status;
  }

  ds402::TransitionStatus handleFault() override {
    ++handle_fault_count;
    return handle_fault_status;
  }

  ds402::TransitionStatus doStateAction(ds402::State current_state) override {
    ++state_action_count;
    last_action_state = current_state;
    return state_action_status;
  }
};

void moveToReadyToSwitchOn(ds402::StateMachine& machine) {
  machine.update();
  machine.setControlWord(kShutdownControlWord);
  machine.update();
}

void moveToSwitchedOn(ds402::StateMachine& machine) {
  moveToReadyToSwitchOn(machine);
  machine.setControlWord(kSwitchOnControlWord);
  machine.update();
}

void moveToOperationEnabled(ds402::StateMachine& machine) {
  moveToSwitchedOn(machine);
  machine.setControlWord(kEnableOperationControlWord);
  machine.update();
}

void moveToQuickStopActive(ds402::StateMachine& machine) {
  moveToOperationEnabled(machine);
  machine.setControlWord(kQuickStopControlWord);
  machine.update();
}

}  // namespace

TEST_GROUP(DS402StateMachine) {
  FakeDS402Drive drive;
  ds402::StateMachine machine{drive};
};

TEST_GROUP(DS402ModeOfOperation){};

TEST_GROUP(DS402HomingMethod){};

TEST(DS402HomingMethod, FactoriesExposeSupportedHomingValues) {
  LONGS_EQUAL(0, static_cast<int8_t>(ds402::HomingMethod::None()));
  LONGS_EQUAL(33, static_cast<int8_t>(ds402::HomingMethod::IndexPositive()));
  LONGS_EQUAL(34, static_cast<int8_t>(ds402::HomingMethod::IndexNegative()));
  LONGS_EQUAL(-127, static_cast<int8_t>(ds402::HomingMethod::Unknown()));
}

TEST(DS402HomingMethod, FromRawValueMapsKnownValuesAndRejectsUnknownValues) {
  CHECK_TRUE(ds402::HomingMethod::FromRawValue(0) ==
             ds402::HomingMethod::None());
  CHECK_TRUE(ds402::HomingMethod::FromRawValue(33) ==
             ds402::HomingMethod::IndexPositive());
  CHECK_TRUE(ds402::HomingMethod::FromRawValue(34) ==
             ds402::HomingMethod::IndexNegative());
  CHECK_TRUE(ds402::HomingMethod::FromRawValue(35) ==
             ds402::HomingMethod::Unknown());
}

TEST(DS402ModeOfOperation, FactoriesExposeCiA402AndVendorSpecificValues) {
  LONGS_EQUAL(-11, static_cast<int8_t>(
                       ds402::ModeOfOperation::CalibrateSinCosEncoder()));
  LONGS_EQUAL(
      -9, static_cast<int8_t>(ds402::ModeOfOperation::CalibrateAbsEncoder()));
  LONGS_EQUAL(-7, static_cast<int8_t>(
                      ds402::ModeOfOperation::CalibrateIncrementalEncoder()));
  LONGS_EQUAL(-5, static_cast<int8_t>(ds402::ModeOfOperation::MeasureFlux()));
  LONGS_EQUAL(-3,
              static_cast<int8_t>(ds402::ModeOfOperation::MeasureInductance()));
  LONGS_EQUAL(-1,
              static_cast<int8_t>(ds402::ModeOfOperation::MeasureResistance()));
  LONGS_EQUAL(0, static_cast<int8_t>(ds402::ModeOfOperation::None()));
  LONGS_EQUAL(1,
              static_cast<int8_t>(ds402::ModeOfOperation::ProfilePosition()));
  LONGS_EQUAL(3,
              static_cast<int8_t>(ds402::ModeOfOperation::ProfileVelocity()));
  LONGS_EQUAL(4, static_cast<int8_t>(ds402::ModeOfOperation::ProfileTorque()));
  LONGS_EQUAL(6, static_cast<int8_t>(ds402::ModeOfOperation::Homing()));
  LONGS_EQUAL(8, static_cast<int8_t>(
                     ds402::ModeOfOperation::CyclicSynchronousPosition()));
  LONGS_EQUAL(9, static_cast<int8_t>(
                     ds402::ModeOfOperation::CyclicSynchronousVelocity()));
  LONGS_EQUAL(10, static_cast<int8_t>(
                      ds402::ModeOfOperation::CyclicSynchronousTorque()));
}

TEST(DS402ModeOfOperation, FromRawValueMapsKnownValuesAndRejectsUnknownValues) {
  CHECK_TRUE(ds402::ModeOfOperation::FromRawValue(8) ==
             ds402::ModeOfOperation::CyclicSynchronousPosition());
  CHECK_TRUE(ds402::ModeOfOperation::FromRawValue(10) ==
             ds402::ModeOfOperation::CyclicSynchronousTorque());
  CHECK_TRUE(ds402::ModeOfOperation::FromRawValue(-11) ==
             ds402::ModeOfOperation::CalibrateSinCosEncoder());
  CHECK_TRUE(ds402::ModeOfOperation::FromRawValue(2) ==
             ds402::ModeOfOperation::Unknown());
}

TEST(DS402StateMachine,
     InitialUpdateInitializesDriveAndMovesToSwitchOnDisabled) {
  CHECK_TRUE(machine.getState() == ds402::State::NotReadyToSwitchOn());

  const ds402::State state = machine.update();

  CHECK_TRUE(state == ds402::State::SwitchOnDisabled());
  LONGS_EQUAL(1, drive.initialize_count);
  LONGS_EQUAL(1, drive.state_action_count);
  CHECK_TRUE(drive.last_action_state == ds402::State::SwitchOnDisabled());
}

TEST(DS402StateMachine, NormalControlwordSequenceReachesOperationEnabled) {
  machine.update();

  machine.setControlWord(kShutdownControlWord);
  CHECK_TRUE(machine.update() == ds402::State::ReadyToSwitchOn());

  machine.setControlWord(kSwitchOnControlWord);
  CHECK_TRUE(machine.update() == ds402::State::SwitchedOn());
  LONGS_EQUAL(1, drive.enable_power_count);

  machine.setControlWord(kEnableOperationControlWord);
  CHECK_TRUE(machine.update() == ds402::State::OperationEnabled());
  LONGS_EQUAL(1, drive.enable_drive_count);

  const uint16_t expected_status_word =
      static_cast<uint16_t>(ds402::State::OperationEnabled()) |
      ds402::kStatusWordVoltageEnabledMask;
  LONGS_EQUAL(expected_status_word, machine.getStatusWord());
}

TEST(DS402StateMachine, OngoingDriveTransitionHoldsCurrentState) {
  machine.update();
  machine.setControlWord(kShutdownControlWord);
  machine.update();

  drive.enable_power_status = ds402::TransitionStatus::kOngoing;
  machine.setControlWord(kSwitchOnControlWord);

  CHECK_TRUE(machine.update() == ds402::State::ReadyToSwitchOn());
  LONGS_EQUAL(1, drive.enable_power_count);
}

TEST(DS402StateMachine, OngoingEnableDriveHoldsSwitchedOn) {
  moveToSwitchedOn(machine);

  drive.enable_drive_status = ds402::TransitionStatus::kOngoing;
  machine.setControlWord(kEnableOperationControlWord);

  CHECK_TRUE(machine.update() == ds402::State::SwitchedOn());
  LONGS_EQUAL(1, drive.enable_drive_count);

  drive.enable_drive_status = ds402::TransitionStatus::kFinished;
  machine.setControlWord(kEnableOperationControlWord);

  CHECK_TRUE(machine.update() == ds402::State::OperationEnabled());
  LONGS_EQUAL(2, drive.enable_drive_count);
}

TEST(DS402StateMachine, DriveFaultMovesThroughFaultReactionToFault) {
  machine.update();
  machine.setControlWord(kShutdownControlWord);
  machine.update();

  drive.enable_power_status = ds402::TransitionStatus::kFault;
  machine.setControlWord(kSwitchOnControlWord);

  CHECK_TRUE(machine.update() == ds402::State::FaultReactionActive());

  drive.enable_power_status = ds402::TransitionStatus::kFinished;
  CHECK_TRUE(machine.update() == ds402::State::Fault());
  LONGS_EQUAL(1, drive.handle_fault_count);
}

TEST(DS402StateMachine, StateActionFaultMovesToFaultReactionActive) {
  machine.update();
  drive.state_action_status = ds402::TransitionStatus::kFault;

  CHECK_TRUE(machine.update() == ds402::State::FaultReactionActive());
  LONGS_EQUAL(2, drive.state_action_count);

  drive.state_action_status = ds402::TransitionStatus::kFinished;
  CHECK_TRUE(machine.update() == ds402::State::Fault());
  LONGS_EQUAL(1, drive.handle_fault_count);
}

TEST(DS402StateMachine, FaultResetRequiresRisingEdgeOfControlwordBit7) {
  machine.update();
  machine.requestFault();
  CHECK_TRUE(machine.update() == ds402::State::Fault());

  machine.setControlWord(kFaultResetControlWord);
  CHECK_TRUE(machine.update() == ds402::State::SwitchOnDisabled());

  machine.requestFault();
  CHECK_TRUE(machine.update() == ds402::State::Fault());

  machine.setControlWord(kFaultResetControlWord);
  CHECK_TRUE(machine.update() == ds402::State::Fault());

  machine.setControlWord(kDisableVoltageControlWord);
  machine.update();
  machine.setControlWord(kFaultResetControlWord);
  CHECK_TRUE(machine.update() == ds402::State::SwitchOnDisabled());
}

TEST(DS402StateMachine, QuickStopFromOperationEnabledActivatesQuickStop) {
  moveToOperationEnabled(machine);

  machine.setControlWord(kQuickStopControlWord);

  CHECK_TRUE(machine.update() == ds402::State::QuickStopActive());
  LONGS_EQUAL(1, drive.quick_stop_count);
}

TEST(DS402StateMachine,
     DisableOperationFromOperationEnabledReturnsToSwitchedOn) {
  moveToOperationEnabled(machine);

  machine.setControlWord(kSwitchOnControlWord);

  CHECK_TRUE(machine.update() == ds402::State::SwitchedOn());
  LONGS_EQUAL(1, drive.disable_drive_count);
}

TEST(DS402StateMachine, ShutdownFromOperationEnabledReturnsToReadyToSwitchOn) {
  moveToOperationEnabled(machine);

  machine.setControlWord(kShutdownControlWord);

  CHECK_TRUE(machine.update() == ds402::State::ReadyToSwitchOn());
  LONGS_EQUAL(1, drive.disable_drive_count);
  LONGS_EQUAL(1, drive.disable_power_count);
}

TEST(DS402StateMachine,
     DisableVoltageFromOperationEnabledReturnsToSwitchOnDisabled) {
  moveToOperationEnabled(machine);

  machine.setControlWord(kDisableVoltageControlWord);

  CHECK_TRUE(machine.update() == ds402::State::SwitchOnDisabled());
  LONGS_EQUAL(1, drive.disable_drive_count);
  LONGS_EQUAL(1, drive.disable_power_count);
}

TEST(DS402StateMachine,
     DisableVoltageFromReadyToSwitchOnReturnsToSwitchOnDisabled) {
  moveToReadyToSwitchOn(machine);

  machine.setControlWord(kDisableVoltageControlWord);

  CHECK_TRUE(machine.update() == ds402::State::SwitchOnDisabled());
  LONGS_EQUAL(1, drive.disable_power_count);
}

TEST(DS402StateMachine,
     EnableOperationFromQuickStopActiveReturnsToOperationEnabled) {
  moveToQuickStopActive(machine);

  machine.setControlWord(kEnableOperationControlWord);

  CHECK_TRUE(machine.update() == ds402::State::OperationEnabled());
  LONGS_EQUAL(2, drive.enable_drive_count);
}

TEST(DS402StateMachine,
     DisableVoltageFromQuickStopActiveReturnsToSwitchOnDisabled) {
  moveToQuickStopActive(machine);

  machine.setControlWord(kDisableVoltageControlWord);

  CHECK_TRUE(machine.update() == ds402::State::SwitchOnDisabled());
  LONGS_EQUAL(1, drive.disable_drive_count);
  LONGS_EQUAL(1, drive.disable_power_count);
}

TEST(DS402StateMachine, StatusWordVoltageEnabledBitMatchesPoweredStates) {
  machine.update();
  LONGS_EQUAL(static_cast<uint16_t>(ds402::State::SwitchOnDisabled()),
              machine.getStatusWord());

  machine.setControlWord(kShutdownControlWord);
  machine.update();
  LONGS_EQUAL(static_cast<uint16_t>(ds402::State::ReadyToSwitchOn()),
              machine.getStatusWord());

  machine.setControlWord(kSwitchOnControlWord);
  machine.update();
  LONGS_EQUAL(static_cast<uint16_t>(ds402::State::SwitchedOn()) |
                  ds402::kStatusWordVoltageEnabledMask,
              machine.getStatusWord());

  machine.setControlWord(kEnableOperationControlWord);
  machine.update();
  LONGS_EQUAL(static_cast<uint16_t>(ds402::State::OperationEnabled()) |
                  ds402::kStatusWordVoltageEnabledMask,
              machine.getStatusWord());

  machine.setControlWord(kQuickStopControlWord);
  machine.update();
  LONGS_EQUAL(static_cast<uint16_t>(ds402::State::QuickStopActive()) |
                  ds402::kStatusWordVoltageEnabledMask,
              machine.getStatusWord());

  machine.requestFault();
  LONGS_EQUAL(static_cast<uint16_t>(ds402::State::FaultReactionActive()) |
                  ds402::kStatusWordVoltageEnabledMask,
              machine.getStatusWord());

  machine.update();
  LONGS_EQUAL(static_cast<uint16_t>(ds402::State::Fault()),
              machine.getStatusWord());
}

TEST(DS402StateMachine, InvalidCommandDoesNotChangeState) {
  machine.update();

  machine.setControlWord(kEnableOperationControlWord);

  CHECK_TRUE(machine.update() == ds402::State::SwitchOnDisabled());
  LONGS_EQUAL(0, drive.enable_power_count);
  LONGS_EQUAL(0, drive.enable_drive_count);
}

TEST(DS402StateMachine, FaultIgnoresCommandsExceptFaultReset) {
  machine.update();
  machine.requestFault();
  CHECK_TRUE(machine.update() == ds402::State::Fault());

  machine.setControlWord(kShutdownControlWord);
  CHECK_TRUE(machine.update() == ds402::State::Fault());

  machine.setControlWord(kSwitchOnControlWord);
  CHECK_TRUE(machine.update() == ds402::State::Fault());

  machine.setControlWord(kEnableOperationControlWord);
  CHECK_TRUE(machine.update() == ds402::State::Fault());
}

TEST(DS402StateMachine, FaultHandlingOngoingHoldsFaultReactionActive) {
  machine.update();
  machine.requestFault();
  drive.handle_fault_status = ds402::TransitionStatus::kOngoing;

  CHECK_TRUE(machine.update() == ds402::State::FaultReactionActive());
  LONGS_EQUAL(1, drive.handle_fault_count);

  drive.handle_fault_status = ds402::TransitionStatus::kFinished;
  CHECK_TRUE(machine.update() == ds402::State::Fault());
  LONGS_EQUAL(2, drive.handle_fault_count);
}
