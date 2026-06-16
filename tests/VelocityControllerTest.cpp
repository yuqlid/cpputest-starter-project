#include "CppUTest/TestHarness.h"

#include <cmath>

#include "pid_controller.hpp"
#include "velocity_controller.hpp"

TEST_GROUP(PIDController)
{
};

TEST(PIDController, ValidatesOutputLimits)
{
    CHECK_TRUE((PIDControllerConfig_t<float>::isConfigValid(
        {.kp = 1.0f,
         .kd = 0.0f,
         .ki = 0.0f,
         .kaw = 0.0f,
         .output_min = -2.0f,
         .output_max = 2.0f})));

    CHECK_FALSE((PIDControllerConfig_t<float>::isConfigValid(
        {.kp = 1.0f,
         .kd = 0.0f,
         .ki = 0.0f,
         .kaw = 0.0f,
         .output_min = 2.0f,
         .output_max = -2.0f})));

    CHECK_FALSE((PIDControllerConfig_t<float>::isConfigValid(
        {.kp = 1.0f,
         .kd = 0.0f,
         .ki = 0.0f,
         .kaw = 0.0f,
         .output_min = -2.0f,
         .output_max = NAN})));
}

TEST(PIDController, IntegralTermUsesDt)
{
    PIDController<float> pid({.kp = 0.0f,
                              .kd = 0.0f,
                              .ki = 2.0f,
                              .kaw = 0.0f,
                              .output_min = -100.0f,
                              .output_max = 100.0f});

    DOUBLES_EQUAL(0.5, pid.process(1.0f, 0.0f, 0.0f, 0.0f, 0.25f), 1e-6);
    DOUBLES_EQUAL(1.0, pid.process(1.0f, 0.0f, 0.0f, 0.0f, 0.25f), 1e-6);
    DOUBLES_EQUAL(1.0, pid.getITerm(), 1e-6);
}

TEST(PIDController, OutputLimitClampsAndBackCalculatesWithDt)
{
    PIDController<float> pid({.kp = 10.0f,
                              .kd = 0.0f,
                              .ki = 0.0f,
                              .kaw = 1.0f,
                              .output_min = -2.0f,
                              .output_max = 2.0f});

    DOUBLES_EQUAL(2.0, pid.process(1.0f, 0.0f, 0.0f, 0.0f, 0.1f), 1e-6);
    DOUBLES_EQUAL(10.0, pid.getRawOutput(), 1e-6);
    DOUBLES_EQUAL(-0.8, pid.getBackCalculation(), 1e-6);
    DOUBLES_EQUAL(-0.8, pid.getITerm(), 1e-6);
}

TEST(PIDController, InvalidDtDoesNotUpdateState)
{
    PIDController<float> pid({.kp = 1.0f,
                              .kd = 0.0f,
                              .ki = 1.0f,
                              .kaw = 0.0f,
                              .output_min = -10.0f,
                              .output_max = 10.0f});

    DOUBLES_EQUAL(1.1, pid.process(1.0f, 0.0f, 0.0f, 0.0f, 0.1f), 1e-6);

    DOUBLES_EQUAL(1.1, pid.process(10.0f, 0.0f, 0.0f, 0.0f, NAN), 1e-6);
    DOUBLES_EQUAL(1.1, pid.getRawOutput(), 1e-6);
    DOUBLES_EQUAL(0.1, pid.getITerm(), 1e-6);
}

TEST_GROUP(VelocityController)
{
};

TEST(VelocityController, GeneratesQAxisCurrentFromVelocityError)
{
    VelocityController<float> controller(
        {.pid = {.kp = 0.5f,
                 .kd = 0.0f,
                 .ki = 0.0f,
                 .kaw = 0.0f,
                 .output_min = -2.0f,
                 .output_max = 2.0f}});

    controller.setTargetVelocity(10.0f);

    DOUBLES_EQUAL(2.0, controller.update(4.0f, 0.001f), 1e-6);
    DOUBLES_EQUAL(10.0, controller.getState().target_velocity_rad_per_sec, 1e-6);
    DOUBLES_EQUAL(4.0, controller.getState().actual_velocity_rad_per_sec, 1e-6);
    DOUBLES_EQUAL(2.0, controller.getState().target_q_current, 1e-6);
}

TEST(VelocityController, IntegralGainUsesUpdateDt)
{
    VelocityController<float> controller(
        {.pid = {.kp = 0.0f,
                 .kd = 0.0f,
                 .ki = 10.0f,
                 .kaw = 0.0f,
                 .output_min = -100.0f,
                 .output_max = 100.0f}});

    controller.setTargetVelocity(1.0f);

    DOUBLES_EQUAL(1.0, controller.update(0.0f, 0.1f), 1e-6);
    DOUBLES_EQUAL(2.0, controller.update(0.0f, 0.1f), 1e-6);
}

TEST(VelocityController, RejectsInvalidPidConfigWithoutChangingConfig)
{
    VelocityController<float> controller(
        {.pid = {.kp = 1.0f,
                 .kd = 0.0f,
                 .ki = 0.0f,
                 .kaw = 0.0f,
                 .output_min = -3.0f,
                 .output_max = 3.0f}});

    const VelocityControllerConfig<float> before = controller.getConfig();

    CHECK_FALSE(controller.setConfig(
        {.pid = {.kp = 1.0f,
                 .kd = 0.0f,
                 .ki = 0.0f,
                 .kaw = 0.0f,
                 .output_min = 3.0f,
                 .output_max = -3.0f}}));

    const VelocityControllerConfig<float> after = controller.getConfig();
    DOUBLES_EQUAL(before.pid.output_min, after.pid.output_min, 1e-6);
    DOUBLES_EQUAL(before.pid.output_max, after.pid.output_max, 1e-6);
}
