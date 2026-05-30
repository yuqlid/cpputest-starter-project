#include "CppUTest/TestHarness.h"

#include <array>
#include <cmath>
#include <fstream>

#include "TestOutputPath.hpp"
#include "controller/observer.hpp"

TEST_GROUP(MorimotoObserver)
{
    const char* csv_file_name = nullptr;

    void teardown()
    {
        if (csv_file_name != nullptr) {
            TestOutputPath::copyToLatest("MorimotoObserver", csv_file_name);
        }
    }
};

namespace {

constexpr float kPi = 3.14159265358979323846264338327950288f;
constexpr float kTwoPi = 2.0f * kPi;
constexpr float kSamplingFrequencyHz = 20000.0f;
constexpr float kObserverBandwidth = 300.0f;

template <typename T>
T wrapAngle(T input)
{
    constexpr T two_pi = static_cast<T>(kTwoPi);
    T temp = input;
    while (temp < -static_cast<T>(kPi)) {
        temp += two_pi;
    }
    while (temp > static_cast<T>(kPi)) {
        temp -= two_pi;
    }
    return temp;
}

void writeObserverResponseCsv(const char* file_name, const std::array<float, 1024>& theta_meas)
{
    morimoto_observer::Observer<float> observer(
        kObserverBandwidth, 100.0f, 1.0f / kSamplingFrequencyHz);

    const auto path = TestOutputPath::makeCsvPath("MorimotoObserver", file_name);
    std::ofstream csv(path);
    CHECK_TEXT(csv.is_open(), "observer response csv open failed");

    csv << "time_s,theta_in,theta_morimoto,vel_morimoto\n";
    for (size_t i = 0; i < theta_meas.size(); ++i) {
        observer.update(theta_meas.at(i));
        csv << static_cast<float>(i) / kSamplingFrequencyHz << ","
            << theta_meas.at(i) << "," << observer.getPos() << ","
            << observer.getVel() << "\n";
    }
}

}  // namespace

TEST(MorimotoObserver, ValidatesConfigValues)
{
    const auto default_config = ObserverConfig_t<float>::createDefault(20000.0f);
    CHECK_TRUE(ObserverConfig_t<float>::isBandwidthValid(300.0f));
    CHECK_TRUE(ObserverConfig_t<float>::isDampingRatioValid(1.0f));
    CHECK_TRUE(
        ObserverConfig_t<float>::isVelocityBandwidthValid(20000.0f, 100.0f));
    CHECK_TRUE(ObserverConfig_t<float>::isConfigValid(
        {.bandwidth = 300.0f,
         .damping_ratio = 1.0f,
         .velocity_lpf_cutoff_hz = 100.0f},
        20000.0f));

    CHECK_FALSE(ObserverConfig_t<float>::isBandwidthValid(INFINITY));
    CHECK_FALSE(ObserverConfig_t<float>::isBandwidthValid(0.0f));
    CHECK_FALSE(ObserverConfig_t<float>::isDampingRatioValid(NAN));
    CHECK_FALSE(ObserverConfig_t<float>::isDampingRatioValid(0.0f));
    CHECK_FALSE(
        ObserverConfig_t<float>::isVelocityBandwidthValid(20000.0f, 10001.0f));
    CHECK_FALSE(ObserverConfig_t<float>::isConfigValid(
        {.bandwidth = 300.0f,
         .damping_ratio = 1.0f,
         .velocity_lpf_cutoff_hz = 10001.0f},
        20000.0f));
    DOUBLES_EQUAL(1000.0, default_config.bandwidth, 1e-6);
    DOUBLES_EQUAL(1.0, default_config.damping_ratio, 1e-6);
    DOUBLES_EQUAL(200.0, default_config.velocity_lpf_cutoff_hz, 1e-6);
}

TEST(MorimotoObserver, RejectsInvalidObserverBandwidth)
{
    morimoto_observer::Observer<float> observer(300.0f, 100.0f, 0.00005f);
    const ObserverConfig_t<float> before = observer.getConfig();

    CHECK_FALSE(observer.updateBandwidth(INFINITY));
    CHECK_FALSE(observer.updateBandwidth(0.0f));
    CHECK_FALSE(observer.updateBandwidth(300.0f, NAN));
    CHECK_FALSE(observer.updateBandwidth(300.0f, 0.0f));

    const auto after = observer.getConfig();
    DOUBLES_EQUAL(before.bandwidth, after.bandwidth, 1e-6);
    DOUBLES_EQUAL(before.damping_ratio, after.damping_ratio, 1e-6);
    DOUBLES_EQUAL(before.velocity_lpf_cutoff_hz,
                  after.velocity_lpf_cutoff_hz, 1e-6);
}

TEST(MorimotoObserver, RejectsInvalidVelocityBandwidthWithoutChangingConfig)
{
    morimoto_observer::Observer<float> observer(300.0f, 100.0f, 0.00005f);
    const ObserverConfig_t<float> before = observer.getConfig();

    CHECK_FALSE(observer.updateVelocityLpfCutoffHz(NAN));
    CHECK_FALSE(observer.updateVelocityLpfCutoffHz(10001.0f));

    const auto after = observer.getConfig();
    DOUBLES_EQUAL(before.bandwidth, after.bandwidth, 1e-6);
    DOUBLES_EQUAL(before.damping_ratio, after.damping_ratio, 1e-6);
    DOUBLES_EQUAL(before.velocity_lpf_cutoff_hz,
                  after.velocity_lpf_cutoff_hz, 1e-6);
}

TEST(MorimotoObserver, SetConfigRejectsInvalidConfigWithoutChangingConfig)
{
    morimoto_observer::Observer<float> observer(300.0f, 100.0f, 0.00005f);
    const ObserverConfig_t<float> before = observer.getConfig();

    CHECK_FALSE(observer.setConfig({.bandwidth = INFINITY,
                                    .damping_ratio = 1.0f,
                                    .velocity_lpf_cutoff_hz = 100.0f}));
    CHECK_FALSE(observer.setConfig({.bandwidth = 300.0f,
                                    .damping_ratio = 0.0f,
                                    .velocity_lpf_cutoff_hz = 100.0f}));
    CHECK_FALSE(observer.setConfig({.bandwidth = 300.0f,
                                    .damping_ratio = 1.0f,
                                    .velocity_lpf_cutoff_hz = 10001.0f}));

    const auto after = observer.getConfig();
    DOUBLES_EQUAL(before.bandwidth, after.bandwidth, 1e-6);
    DOUBLES_EQUAL(before.damping_ratio, after.damping_ratio, 1e-6);
    DOUBLES_EQUAL(before.velocity_lpf_cutoff_hz,
                  after.velocity_lpf_cutoff_hz, 1e-6);
}

TEST(MorimotoObserver, SetConfigAppliesValidConfig)
{
    morimoto_observer::Observer<float> observer(300.0f, 100.0f, 0.00005f);

    CHECK_TRUE(observer.setConfig({.bandwidth = 500.0f,
                                   .damping_ratio = 0.7f,
                                   .velocity_lpf_cutoff_hz = 200.0f}));

    const auto config = observer.getConfig();
    DOUBLES_EQUAL(500.0, config.bandwidth, 1e-6);
    DOUBLES_EQUAL(0.7, config.damping_ratio, 1e-6);
    DOUBLES_EQUAL(200.0, config.velocity_lpf_cutoff_hz, 1e-6);
}

TEST(MorimotoObserver, ConstructorSanitizesInvalidArguments)
{
    morimoto_observer::Observer<float> observer(INFINITY, INFINITY, NAN);

    const auto config = observer.getConfig();
    DOUBLES_EQUAL(1000.0, config.bandwidth, 1e-6);
    DOUBLES_EQUAL(1.0, config.damping_ratio, 1e-6);
    DOUBLES_EQUAL(200.0, config.velocity_lpf_cutoff_hz, 1e-6);

    observer.update(1.0f);

    CHECK_TRUE(std::isfinite(observer.getPos()));
    CHECK_TRUE(std::isfinite(observer.getVel()));
}

TEST(MorimotoObserver, UpdateAdvancesObserverState)
{
    morimoto_observer::Observer<float> observer(300.0f, 100.0f, 0.00005f);

    observer.update(1.0f);

    CHECK_TRUE(observer.getPos() > 0.0f);
}

TEST(MorimotoObserver, ResetClearsObserverState)
{
    morimoto_observer::Observer<float> observer(300.0f, 100.0f, 0.00005f);

    observer.update(1.0f);
    observer.reset();

    DOUBLES_EQUAL(0.0, observer.getPos(), 1e-6);
    DOUBLES_EQUAL(0.0, observer.getVel(), 1e-6);
}

TEST(MorimotoObserver, WritesStepResponseCsv)
{
    csv_file_name = "step_response.csv";
    std::array<float, 1024> theta_meas{};

    for (size_t i = 0; i < theta_meas.size(); ++i) {
        theta_meas.at(i) = (i == 0) ? 0.0f : kPi / 2.0f;
    }

    writeObserverResponseCsv(csv_file_name, theta_meas);
}

TEST(MorimotoObserver, WritesSinResponseCsv)
{
    csv_file_name = "sin_response.csv";
    std::array<float, 1024> theta_meas{};

    for (size_t i = 0; i < theta_meas.size(); ++i) {
        theta_meas.at(i) = wrapAngle(std::sin(kTwoPi * static_cast<float>(i) /
                                              static_cast<float>(theta_meas.size())));
    }

    writeObserverResponseCsv(csv_file_name, theta_meas);
}

TEST(MorimotoObserver, WritesRampResponseCsv)
{
    csv_file_name = "ramp_response.csv";
    std::array<float, 1024> theta_meas{};
    constexpr float velocity_rpm = 3000.0f;
    constexpr float velocity_rad_s = velocity_rpm * kPi / 60.0f;

    for (size_t i = 0; i < theta_meas.size(); ++i) {
        theta_meas.at(i) = wrapAngle(static_cast<float>(i) * velocity_rad_s /
                                     kSamplingFrequencyHz);
    }

    writeObserverResponseCsv(csv_file_name, theta_meas);
}
