#include "CppUTest/TestHarness.h"

#include <array>
#include <cmath>
#include <fstream>

#include "TestOutputPath.hpp"
#include "controller/observer.hpp"

TEST_GROUP(MotimoroObserver)
{
    const char* csv_file_name = nullptr;

    void teardown()
    {
        if (csv_file_name != nullptr) {
            TestOutputPath::copyToLatest("MotimoroObserver", csv_file_name);
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
    motimoro_observer::Observer<float> observer(
        kObserverBandwidth, 100.0f, 1.0f / kSamplingFrequencyHz);

    const auto path = TestOutputPath::makeCsvPath("MotimoroObserver", file_name);
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

TEST(MotimoroObserver, ValidatesConfigValues)
{
    CHECK_TRUE(ObserverConfig_t<float>::isBandwidthValid(300.0f));
    CHECK_TRUE(ObserverConfig_t<float>::isDampingRatioValid(1.0f));
    CHECK_TRUE(
        ObserverConfig_t<float>::isVelocityBandwidthValid(20000.0f, 100.0f));
    CHECK_TRUE(ObserverConfig_t<float>::isConfigValid(
        {.bandwitdh = 300.0f, .damping_ratio = 1.0f, .bandwitdh_vel = 100.0f},
        20000.0f));

    CHECK_FALSE(ObserverConfig_t<float>::isBandwidthValid(INFINITY));
    CHECK_FALSE(ObserverConfig_t<float>::isBandwidthValid(0.0f));
    CHECK_FALSE(ObserverConfig_t<float>::isDampingRatioValid(NAN));
    CHECK_FALSE(ObserverConfig_t<float>::isDampingRatioValid(0.0f));
    CHECK_FALSE(
        ObserverConfig_t<float>::isVelocityBandwidthValid(20000.0f, 10001.0f));
    CHECK_FALSE(ObserverConfig_t<float>::isConfigValid(
        {.bandwitdh = 300.0f, .damping_ratio = 1.0f, .bandwitdh_vel = 10001.0f},
        20000.0f));
}

TEST(MotimoroObserver, RejectsInvalidObserverBandwidth)
{
    motimoro_observer::Observer<float> observer(300.0f, 100.0f, 0.00005f);
    const ObserverConfig_t<float> before = observer.getConfig();

    CHECK_FALSE(observer.updateBandWitdh(INFINITY));
    CHECK_FALSE(observer.updateBandWitdh(0.0f));
    CHECK_FALSE(observer.updateBandWitdh(300.0f, NAN));
    CHECK_FALSE(observer.updateBandWitdh(300.0f, 0.0f));

    const auto after = observer.getConfig();
    DOUBLES_EQUAL(before.bandwitdh, after.bandwitdh, 1e-6);
    DOUBLES_EQUAL(before.damping_ratio, after.damping_ratio, 1e-6);
    DOUBLES_EQUAL(before.bandwitdh_vel, after.bandwitdh_vel, 1e-6);
}

TEST(MotimoroObserver, RejectsInvalidVelocityBandwidthWithoutChangingConfig)
{
    motimoro_observer::Observer<float> observer(300.0f, 100.0f, 0.00005f);
    const ObserverConfig_t<float> before = observer.getConfig();

    CHECK_FALSE(observer.updateVelocityBandWidth(NAN));
    CHECK_FALSE(observer.updateVelocityBandWidth(10001.0f));

    const auto after = observer.getConfig();
    DOUBLES_EQUAL(before.bandwitdh, after.bandwitdh, 1e-6);
    DOUBLES_EQUAL(before.damping_ratio, after.damping_ratio, 1e-6);
    DOUBLES_EQUAL(before.bandwitdh_vel, after.bandwitdh_vel, 1e-6);
}

TEST(MotimoroObserver, SetConfigRejectsInvalidConfigWithoutChangingConfig)
{
    motimoro_observer::Observer<float> observer(300.0f, 100.0f, 0.00005f);
    const ObserverConfig_t<float> before = observer.getConfig();

    CHECK_FALSE(observer.setConfig({.bandwitdh = INFINITY,
                                    .damping_ratio = 1.0f,
                                    .bandwitdh_vel = 100.0f}));
    CHECK_FALSE(observer.setConfig({.bandwitdh = 300.0f,
                                    .damping_ratio = 0.0f,
                                    .bandwitdh_vel = 100.0f}));
    CHECK_FALSE(observer.setConfig({.bandwitdh = 300.0f,
                                    .damping_ratio = 1.0f,
                                    .bandwitdh_vel = 10001.0f}));

    const auto after = observer.getConfig();
    DOUBLES_EQUAL(before.bandwitdh, after.bandwitdh, 1e-6);
    DOUBLES_EQUAL(before.damping_ratio, after.damping_ratio, 1e-6);
    DOUBLES_EQUAL(before.bandwitdh_vel, after.bandwitdh_vel, 1e-6);
}

TEST(MotimoroObserver, SetConfigAppliesValidConfig)
{
    motimoro_observer::Observer<float> observer(300.0f, 100.0f, 0.00005f);

    CHECK_TRUE(observer.setConfig({.bandwitdh = 500.0f,
                                   .damping_ratio = 0.7f,
                                   .bandwitdh_vel = 200.0f}));

    const auto config = observer.getConfig();
    DOUBLES_EQUAL(500.0, config.bandwitdh, 1e-6);
    DOUBLES_EQUAL(0.7, config.damping_ratio, 1e-6);
    DOUBLES_EQUAL(200.0, config.bandwitdh_vel, 1e-6);
}

TEST(MotimoroObserver, UpdateAdvancesObserverState)
{
    motimoro_observer::Observer<float> observer(300.0f, 100.0f, 0.00005f);

    observer.update(1.0f);

    CHECK_TRUE(observer.getPos() > 0.0f);
}

TEST(MotimoroObserver, WritesStepResponseCsv)
{
    csv_file_name = "step_response.csv";
    std::array<float, 1024> theta_meas{};

    for (size_t i = 0; i < theta_meas.size(); ++i) {
        theta_meas.at(i) = (i == 0) ? 0.0f : kPi / 2.0f;
    }

    writeObserverResponseCsv(csv_file_name, theta_meas);
}

TEST(MotimoroObserver, WritesSinResponseCsv)
{
    csv_file_name = "sin_response.csv";
    std::array<float, 1024> theta_meas{};

    for (size_t i = 0; i < theta_meas.size(); ++i) {
        theta_meas.at(i) = wrapAngle(std::sin(kTwoPi * static_cast<float>(i) /
                                              static_cast<float>(theta_meas.size())));
    }

    writeObserverResponseCsv(csv_file_name, theta_meas);
}

TEST(MotimoroObserver, WritesRampResponseCsv)
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
