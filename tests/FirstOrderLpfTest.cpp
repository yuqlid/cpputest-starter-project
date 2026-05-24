#include "CppUTest/TestHarness.h"

#include <cmath>
#include <complex>
#include <fstream>

#include "TestOutputPath.hpp"
#include "filter/firstorderlpf.hpp"

TEST_GROUP(FirstOrderLpf)
{
    const char* csv_file_name = nullptr;

    void teardown()
    {
        if (csv_file_name != nullptr) {
            TestOutputPath::copyToLatest("FirstOrderLpf", csv_file_name);
        }
    }
};

namespace {

constexpr float kPi = 3.14159265358979323846264338327950288f;

struct LpfCoefficients {
    float num;
    float den;
};

LpfCoefficients makeLegacyCoefficients(float normalized_cutoff_frequency)
{
    const float k = kPi * normalized_cutoff_frequency / 2.0f;
    return {k / (1.0f + k), (k - 1.0f) / (k + 1.0f)};
}

LpfCoefficients makePrewarpedCoefficients(float normalized_cutoff_frequency)
{
    const float k = std::tan(kPi * normalized_cutoff_frequency / 2.0f);
    return {k / (1.0f + k), (k - 1.0f) / (k + 1.0f)};
}

std::complex<float> frequencyResponse(const LpfCoefficients& coeff,
                                      float frequency_hz,
                                      float fs_hz)
{
    const std::complex<float> jtheta(0.0f, 2.0f * kPi * frequency_hz / fs_hz);
    const std::complex<float> inv_z(std::exp(-jtheta));
    return coeff.num * (1.0f + inv_z) / (1.0f + coeff.den * inv_z);
}

void writeStepResponseCsv(const char* file_name)
{
    constexpr float fs_hz = 20000.0f;
    constexpr float fc_hz = 1000.0f;
    constexpr int period = 256;
    FirstOrderLpf<float> by_frequency(fs_hz, fc_hz);
    FirstOrderLpf<float> by_normalized(2.0f * fc_hz / fs_hz);
    FirstOrderLpf<float> prewarped(2.0f * fc_hz / fs_hz);
    CHECK_TRUE(prewarped.updateBandwidthPrewarped(fs_hz, fc_hz));

    const auto path = TestOutputPath::makeCsvPath("FirstOrderLpf", file_name);
    std::ofstream csv(path);
    CHECK_TEXT(csv.is_open(), "first order lpf step csv open failed");

    csv << "time,input,out_frequency,out_normalized,out_prewarped\n";
    for (int i = 0; i < period; ++i) {
        const float input = (i > 100) ? 1.0f : 0.0f;
        csv << static_cast<float>(i) / fs_hz << "," << input << ","
            << by_frequency.update(input) << "," << by_normalized.update(input)
            << "," << prewarped.update(input) << "\n";
    }
}

void writeTransferFunctionCsv(const char* file_name)
{
    constexpr float fs_hz = 20000.0f;
    constexpr float fc_hz = 2000.0f;
    constexpr uint32_t omega_rads = fs_hz / 2 * 2 * kPi;
    constexpr float normalized_cutoff_frequency = 2.0f * fc_hz / fs_hz;
    const auto legacy = makeLegacyCoefficients(normalized_cutoff_frequency);
    const auto prewarped = makePrewarpedCoefficients(normalized_cutoff_frequency);

    const auto path = TestOutputPath::makeCsvPath("FirstOrderLpf", file_name);
    std::ofstream csv(path);
    CHECK_TEXT(csv.is_open(), "first order lpf transfer csv open failed");

    csv << "frequency_hz,legacy_gain_db,legacy_phase_deg,"
           "prewarped_gain_db,prewarped_phase_deg\n";
    for (uint32_t i = 1; i < omega_rads; ++i) {
        const float frequency_hz = static_cast<float>(i) / (2.0f * kPi);
        const auto legacy_response =
            frequencyResponse(legacy, frequency_hz, fs_hz);
        const auto prewarped_response =
            frequencyResponse(prewarped, frequency_hz, fs_hz);

        csv << frequency_hz << ","
            << 20.0f * std::log10(std::abs(legacy_response)) << ","
            << 180.0f * std::arg(legacy_response) / kPi << ","
            << 20.0f * std::log10(std::abs(prewarped_response)) << ","
            << 180.0f * std::arg(prewarped_response) / kPi << "\n";
    }
}

}  // namespace

TEST(FirstOrderLpf, ValidatesCutoffFrequencies)
{
    CHECK_TRUE(FirstOrderLpf<float>::isSamplingFrequencyValid(20000.0f));
    CHECK_TRUE(FirstOrderLpf<float>::isCutoffFrequencyValid(20000.0f, 0.0f));
    CHECK_TRUE(FirstOrderLpf<float>::isCutoffFrequencyValid(20000.0f, 10000.0f));
    CHECK_TRUE(FirstOrderLpf<float>::isNormalizedCutoffFrequencyValid(1.0f));
    CHECK_TRUE(
        FirstOrderLpf<float>::isPrewarpedCutoffFrequencyValid(20000.0f, 0.0f));
    CHECK_TRUE(FirstOrderLpf<float>::isPrewarpedNormalizedCutoffFrequencyValid(
        0.999f));

    CHECK_FALSE(FirstOrderLpf<float>::isSamplingFrequencyValid(0.0f));
    CHECK_FALSE(FirstOrderLpf<float>::isCutoffFrequencyValid(20000.0f, -1.0f));
    CHECK_FALSE(FirstOrderLpf<float>::isCutoffFrequencyValid(20000.0f, 10001.0f));
    CHECK_FALSE(FirstOrderLpf<float>::isCutoffFrequencyValid(20000.0f, NAN));
    CHECK_FALSE(FirstOrderLpf<float>::isNormalizedCutoffFrequencyValid(1.001f));
    CHECK_FALSE(FirstOrderLpf<float>::isNormalizedCutoffFrequencyValid(INFINITY));
    CHECK_FALSE(FirstOrderLpf<float>::isPrewarpedCutoffFrequencyValid(
        20000.0f, 10000.0f));
    CHECK_FALSE(
        FirstOrderLpf<float>::isPrewarpedNormalizedCutoffFrequencyValid(1.0f));
}

TEST(FirstOrderLpf, RejectsInvalidBandwidthUpdate)
{
    FirstOrderLpf<float> lpf(20000.0f, 1000.0f);

    CHECK_FALSE(lpf.updateBandwidth(0.0f, 1000.0f));
    CHECK_FALSE(lpf.updateBandwidth(20000.0f, 10001.0f));
    CHECK_FALSE(lpf.updateBandwidth(NAN));
    CHECK_FALSE(lpf.updateBandwidthPrewarped(20000.0f, 10000.0f));
    CHECK_FALSE(lpf.updateBandwidthPrewarped(1.0f));
}

TEST(FirstOrderLpf, GetsNormalizedCutoffFrequency)
{
    FirstOrderLpf<float> by_frequency(20000.0f, 1000.0f);
    FirstOrderLpf<float> by_normalized(0.25f);

    DOUBLES_EQUAL(0.1, by_frequency.getNormalizedCutoffFrequency(), 1e-6);
    DOUBLES_EQUAL(0.25, by_normalized.getNormalizedCutoffFrequency(), 1e-6);

    CHECK_TRUE(by_frequency.updateBandwidth(20000.0f, 2000.0f));
    DOUBLES_EQUAL(0.2, by_frequency.getNormalizedCutoffFrequency(), 1e-6);

    CHECK_TRUE(by_normalized.updateBandwidth(0.3f));
    DOUBLES_EQUAL(0.3, by_normalized.getNormalizedCutoffFrequency(), 1e-6);

    CHECK_TRUE(by_frequency.updateBandwidthPrewarped(20000.0f, 3000.0f));
    DOUBLES_EQUAL(0.3, by_frequency.getNormalizedCutoffFrequency(), 1e-6);
}

TEST(FirstOrderLpf, PrewarpedBandwidthUsesDigitalHalfPowerFrequency)
{
    FirstOrderLpf<float> lpf(0.5f);

    CHECK_TRUE(lpf.updateBandwidthPrewarped(0.5f));

    DOUBLES_EQUAL(0.5, lpf.update(1.0f), 1e-6);

    const auto coeff = makePrewarpedCoefficients(0.5f);
    const auto response = frequencyResponse(coeff, 0.25f, 1.0f);
    DOUBLES_EQUAL(1.0 / std::sqrt(2.0), std::abs(response), 1e-6);
}

TEST(FirstOrderLpf, FrequencyAndNormalizedConstructorsHaveSameResponse)
{
    constexpr float fs_hz = 20000.0f;
    constexpr float fc_hz = 1000.0f;
    FirstOrderLpf<float> by_frequency(fs_hz, fc_hz);
    FirstOrderLpf<float> by_normalized(2.0f * fc_hz / fs_hz);

    for (int i = 0; i < 32; ++i) {
        const float input = (i > 4) ? 1.0f : 0.0f;
        DOUBLES_EQUAL(by_frequency.update(input), by_normalized.update(input), 1e-6);
    }
}

TEST(FirstOrderLpf, StepResponseApproachesDcGainWithoutOvershoot)
{
    FirstOrderLpf<float> lpf(20000.0f, 1000.0f);
    float previous = 0.0f;
    float output = 0.0f;

    for (int i = 0; i < 200; ++i) {
        output = lpf.update(1.0f);
        CHECK_TRUE(output >= previous);
        CHECK_TRUE(output <= 1.0f);
        previous = output;
    }

    DOUBLES_EQUAL(1.0, output, 1e-4);
}

TEST(FirstOrderLpf, WritesStepResponseCsv)
{
    csv_file_name = "step_response.csv";
    writeStepResponseCsv(csv_file_name);
}

TEST(FirstOrderLpf, WritesTransferFunctionCsv)
{
    csv_file_name = "transfer_function.csv";
    writeTransferFunctionCsv(csv_file_name);
}
