#include "CppUTest/TestHarness.h"

#include <cmath>
#include <fstream>

#include "TestOutputPath.hpp"
#include "dq_current_controller.hpp"

TEST_GROUP(DqCurrentController)
{
    const char* csv_file_name = nullptr;

    void teardown()
    {
        if (csv_file_name != nullptr) {
            TestOutputPath::copyToLatest("DqCurrentController", csv_file_name);
        }
    }
};

namespace {

float magnitude(const directquadrature_t<float>& value)
{
    return std::sqrt(value.d * value.d + value.q * value.q);
}

void writeVoltageLimitStepResponseCsv(const char* file_name)
{
    constexpr float dt_sec = 0.001f;
    constexpr float max_voltage = 2.0f;
    constexpr int sample_count = 180;

    dqCurrentController<float> controller({
        .kp = {8.0f, 8.0f},
        .ki = {20.0f, 20.0f},
        .kaw = {0.125f, 0.125f},
    });

    const auto path =
        TestOutputPath::makeCsvPath("DqCurrentController", file_name);
    std::ofstream csv(path);
    CHECK_TEXT(csv.is_open(), "dq current controller csv open failed");

    csv << "time_s,ref_d,ref_q,raw_vd,raw_vq,limited_vd,limited_vq,"
           "integrator_d,integrator_q,back_calc_d,back_calc_q,"
           "raw_mag,limited_mag,max_voltage\n";

    for (int i = 0; i < sample_count; ++i) {
        const float time_s = static_cast<float>(i) * dt_sec;
        const directquadrature_t<float> ref = {
            i < 20 ? 0.0f : 1.0f,
            i < 80 ? 0.0f : 0.5f,
        };
        const directquadrature_t<float> fb{0.0f, 0.0f};
        const directquadrature_t<float> feedforward_voltage{
            i < 120 ? 0.0f : 1.2f,
            0.0f,
        };

        const auto output = controller.updateWithVoltageLimit(
            ref, fb, feedforward_voltage, max_voltage, dt_sec);
        const auto raw_output = controller.getRawOutput();
        const auto integrator = controller.getIntegrator();
        const auto back_calculation = controller.getBackCalculation();

        csv << time_s << "," << ref.d << "," << ref.q << ","
            << raw_output.d << "," << raw_output.q << "," << output.d << ","
            << output.q << "," << integrator.d << "," << integrator.q << ","
            << back_calculation.d << "," << back_calculation.q << ","
            << magnitude(raw_output) << "," << magnitude(output) << ","
            << max_voltage << "\n";

        CHECK_TRUE(magnitude(output) <= max_voltage + 1.0e-5f);
    }
}

}  // namespace

TEST(DqCurrentController, IntegralTermUsesDtOnBothAxes)
{
    dqCurrentController<float> controller({
        .kp = {0.0f, 0.0f},
        .ki = {2.0f, 4.0f},
        .kaw = {0.0f, 0.0f},
    });

    const auto output =
        controller.update({1.0f, -1.0f}, {0.0f, 0.0f}, 0.25f);

    DOUBLES_EQUAL(0.5, output.d, 1e-6);
    DOUBLES_EQUAL(-1.0, output.q, 1e-6);

    const auto integrator = controller.getIntegrator();
    DOUBLES_EQUAL(0.5, integrator.d, 1e-6);
    DOUBLES_EQUAL(-1.0, integrator.q, 1e-6);
}

TEST(DqCurrentController, ResetClearsIntegrator)
{
    dqCurrentController<float> controller({
        .kp = {0.0f, 0.0f},
        .ki = {2.0f, 4.0f},
        .kaw = {0.0f, 0.0f},
    });

    controller.update({1.0f, -1.0f}, {0.0f, 0.0f}, 0.25f);
    controller.reset();

    const auto integrator = controller.getIntegrator();
    DOUBLES_EQUAL(0.0, integrator.d, 1e-6);
    DOUBLES_EQUAL(0.0, integrator.q, 1e-6);

    const auto output =
        controller.update({0.0f, 0.0f}, {0.0f, 0.0f}, 0.25f);
    DOUBLES_EQUAL(0.0, output.d, 1e-6);
    DOUBLES_EQUAL(0.0, output.q, 1e-6);
}

TEST(DqCurrentController, LimitVoltageVectorScalesDqMagnitude)
{
    const auto limited =
        dqCurrentController<float>::limitVoltageVector({3.0f, 4.0f}, 2.5f);

    DOUBLES_EQUAL(1.5, limited.d, 1e-6);
    DOUBLES_EQUAL(2.0, limited.q, 1e-6);
}

TEST(DqCurrentController, VoltageLimitBackCalculatesIntegrator)
{
    dqCurrentController<float> controller({
        .kp = {10.0f, 10.0f},
        .ki = {0.0f, 0.0f},
        .kaw = {1.0f, 1.0f},
    });

    const auto output = controller.updateWithVoltageLimit(
        {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 2.0f, 0.1f);

    DOUBLES_EQUAL(2.0, output.d, 1e-6);
    DOUBLES_EQUAL(0.0, output.q, 1e-6);

    const auto raw_output = controller.getRawOutput();
    DOUBLES_EQUAL(10.0, raw_output.d, 1e-6);
    DOUBLES_EQUAL(0.0, raw_output.q, 1e-6);

    const auto back_calculation = controller.getBackCalculation();
    DOUBLES_EQUAL(-0.8, back_calculation.d, 1e-6);
    DOUBLES_EQUAL(0.0, back_calculation.q, 1e-6);

    const auto integrator = controller.getIntegrator();
    DOUBLES_EQUAL(-0.8, integrator.d, 1e-6);
    DOUBLES_EQUAL(0.0, integrator.q, 1e-6);
}

TEST(DqCurrentController, VoltageLimitIncludesFeedforwardVoltage)
{
    dqCurrentController<float> controller({
        .kp = {0.0f, 0.0f},
        .ki = {0.0f, 0.0f},
        .kaw = {0.0f, 0.0f},
    });

    const auto output = controller.updateWithVoltageLimit(
        {0.0f, 0.0f}, {0.0f, 0.0f}, {3.0f, 4.0f}, 2.5f, 0.1f);

    DOUBLES_EQUAL(1.5, output.d, 1e-6);
    DOUBLES_EQUAL(2.0, output.q, 1e-6);

    const auto raw_output = controller.getRawOutput();
    DOUBLES_EQUAL(3.0, raw_output.d, 1e-6);
    DOUBLES_EQUAL(4.0, raw_output.q, 1e-6);
}

TEST(DqCurrentController, WritesVoltageLimitStepResponseCsv)
{
    csv_file_name = "voltage_limit_step_response.csv";
    writeVoltageLimitStepResponseCsv(csv_file_name);
}
