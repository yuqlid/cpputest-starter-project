#include "CppUTest/TestHarness.h"

#include <cmath>
#include <cstdint>

#include "angle_mapper.hpp"
#include "motor/motor.hpp"
#include "motor/motor_damiao_j3507.hpp"
#include "motor/motor_dji_m2006.hpp"
#include "motor/motor_maxon_ex4pole.hpp"
#include "motor_driver/drv8316cr_util.hpp"
#include "ringbufwithdma.hpp"
#include "utils/atan2.hpp"

namespace {

template <typename T>
T pi()
{
    return static_cast<T>(3.14159265358979323846);
}

}  // namespace

TEST_GROUP(AngleMapper)
{
};

TEST(AngleMapper, DefaultConfigIsIdentity)
{
    const auto config = AngleMappingConfig_t<float>::createDefault();
    const AngleMapper<float> mapper(config);

    CHECK_TRUE(AngleMappingConfig_t<float>::isConfigValid(config));
    DOUBLES_EQUAL(2.5, mapper.convert(2.5f), 0.000001);
    DOUBLES_EQUAL(1.0, mapper.getGain(), 0.000001);
    DOUBLES_EQUAL(0.0, mapper.getOffset(), 0.000001);
    DOUBLES_EQUAL(1.0, mapper.getDir(), 0.000001);
}

TEST(AngleMapper, NegativeGainUpdatesDirection)
{
    AngleMapper<float> mapper({2.0f, 1.0f});

    mapper.setConfig({-0.5f, 0.25f});

    DOUBLES_EQUAL(-1.0, mapper.getDir(), 0.000001);
    DOUBLES_EQUAL(-1.75, mapper.convert(4.0f), 0.000001);
}

TEST(AngleMapper, InvalidConfigIsRejected)
{
    AngleMapper<float> mapper({2.0f, 1.0f});

    mapper.setConfig({NAN, 5.0f});

    DOUBLES_EQUAL(2.0, mapper.getGain(), 0.000001);
    DOUBLES_EQUAL(1.0, mapper.getOffset(), 0.000001);
}

TEST_GROUP(MotorConfig)
{
};

TEST(MotorConfig, DefaultAndKnownMotorParamsAreValid)
{
    CHECK_TRUE(Motor<float>::isConfigValid(Motor<float>::createDefault()));
    CHECK_TRUE(Motor<float>::isConfigValid(motor_params::dji_m2006));
    CHECK_TRUE(Motor<float>::isConfigValid(motor_params::damiao_j3507));
    CHECK_TRUE(Motor<float>::isConfigValid(motor_params::maxon_ec4pole));
}

TEST(MotorConfig, RejectsZeroNegativeAndNonFiniteValues)
{
    CHECK_FALSE(Motor<float>::isConfigValid({0, 0.1f, 0.0001f, 0.01f}));
    CHECK_FALSE(Motor<float>::isConfigValid({7, 0.0f, 0.0001f, 0.01f}));
    CHECK_FALSE(Motor<float>::isConfigValid({7, -0.1f, 0.0001f, 0.01f}));
    CHECK_FALSE(Motor<float>::isConfigValid({7, 0.1f, INFINITY, 0.01f}));
}

TEST_GROUP(RingBufferWithDma)
{
};

TEST(RingBufferWithDma, ReportsAvailableDataFromDmaCounter)
{
    volatile uint32_t dmaCounter = 8;
    ringBufferWithDma<uint8_t, 8> ring(dmaCounter);

    CHECK_TRUE(ring.isEmpty());

    dmaCounter = 5;
    LONGS_EQUAL(3, ring.getAvailableDataSize());
    CHECK_FALSE(ring.isEmpty());
    CHECK_FALSE(ring.isFull());

    dmaCounter = 0;
    CHECK_TRUE(ring.isEmpty());
}

TEST(RingBufferWithDma, DequeueAdvancesTailAndWraps)
{
    volatile uint32_t dmaCounter = 0;
    ringBufferWithDma<uint8_t, 4> ring(dmaCounter);
    uint8_t* buffer = ring.bufferPointer();
    buffer[0] = 10;
    buffer[1] = 11;
    buffer[2] = 12;
    buffer[3] = 13;

    dmaCounter = 1;
    LONGS_EQUAL(3, ring.getAvailableDataSize());
    LONGS_EQUAL(10, ring.dequeue());
    LONGS_EQUAL(11, ring.dequeue());
    LONGS_EQUAL(1, ring.getAvailableDataSize());

    dmaCounter = 3;
    LONGS_EQUAL(3, ring.getAvailableDataSize());
    LONGS_EQUAL(12, ring.dequeue());
    LONGS_EQUAL(13, ring.dequeue());
    LONGS_EQUAL(10, ring.dequeue());
}

TEST_GROUP(Atan2Approx)
{
};

TEST(Atan2Approx, StdWrapperMatchesStdAtan2)
{
    DOUBLES_EQUAL(std::atan2(1.0, 1.0), Atan2Std::eval(1.0, 1.0), 0.000001);
    DOUBLES_EQUAL(std::atan2(1.0, -1.0), Atan2Std::eval(1.0, -1.0), 0.000001);
    DOUBLES_EQUAL(std::atan2(-1.0, -1.0), Atan2Std::eval(-1.0, -1.0), 0.000001);
}

TEST(Atan2Approx, HandlesAxesAndQuadrants)
{
    DOUBLES_EQUAL(pi<double>() / 2.0, Atan2Rajan9::eval(1.0, 0.0), 0.000001);
    DOUBLES_EQUAL(-pi<double>() / 2.0, Atan2Rajan9::eval(-1.0, 0.0), 0.000001);
    DOUBLES_EQUAL(0.0, Atan2Rajan9::eval(0.0, 0.0), 0.000001);
    DOUBLES_EQUAL(pi<double>() * 3.0 / 4.0, Atan2Rajan9::eval(1.0, -1.0), 0.02);
    DOUBLES_EQUAL(-pi<double>() * 3.0 / 4.0, Atan2Hastings::eval(-1.0, -1.0), 0.02);
}

TEST(Atan2Approx, StaysCloseToStdAtan2)
{
    DOUBLES_EQUAL(std::atan2(0.25, 0.75), Atan2Rajan7::eval(0.25, 0.75), 0.01);
    DOUBLES_EQUAL(std::atan2(0.75, 0.25), Atan2Rajan9::eval(0.75, 0.25), 0.01);
    DOUBLES_EQUAL(std::atan2(-0.4, 0.9), Atan2Hastings::eval(-0.4, 0.9), 0.01);
}

TEST_GROUP(Drv8316crRegisters)
{
};

TEST(Drv8316crRegisters, BuildCtrlWordSetsReadWriteAddressDataAndParity)
{
    using namespace drv8316cr;

    const uint16_t writeWord = buildCtrlWord(CtrlMode::kWrite, Map::kControl2, 0x5Au);
    const uint16_t readWord = buildCtrlWord(CtrlMode::kRead, Map::kControl2, 0x123u);

    UNSIGNED_LONGS_EQUAL(0x095Au, writeWord);
    UNSIGNED_LONGS_EQUAL(0x8923u, readWord);
}

TEST(Drv8316crRegisters, Ctrl04PacksAndUnpacksFields)
{
    drv8316cr::Ctrl04::Ctrl04_t reg{};
    reg.clr_flt = true;
    reg.pwm_mode = drv8316cr::Ctrl04::PwmMode_e::kPwm3xModeLimit;
    reg.slew = drv8316cr::Ctrl04::SlewRate_e::kSlewRate150;
    reg.sdo_mode = drv8316cr::Ctrl04::SdoMode_e::kSdoPushPull;
    reg.reserved1 = false;
    reg.reserved2 = true;

    const uint8_t value = drv8316cr::Ctrl04::getCtrlReg04Value(reg);

    UNSIGNED_LONGS_EQUAL(0xB7u, value);

    drv8316cr::Ctrl04::Ctrl04_t decoded{};
    drv8316cr::Ctrl04::getCtrl04FromValue(value, decoded);
    CHECK_TRUE(decoded.clr_flt);
    CHECK_TRUE(decoded.pwm_mode == drv8316cr::Ctrl04::PwmMode_e::kPwm3xModeLimit);
    CHECK_TRUE(decoded.slew == drv8316cr::Ctrl04::SlewRate_e::kSlewRate150);
    CHECK_TRUE(decoded.sdo_mode == drv8316cr::Ctrl04::SdoMode_e::kSdoPushPull);
    CHECK_FALSE(decoded.reserved1);
    CHECK_TRUE(decoded.reserved2);
}

TEST(Drv8316crRegisters, BuildAndReadRegisterMapRoundTripsControlRegisters)
{
    drv8316cr::RegisterMap regMap{};
    regMap.ctrl03.reg_lock = drv8316cr::Ctrl03::RegLock_e::kUnlockAll;
    regMap.ctrl04.pwm_mode = drv8316cr::Ctrl04::PwmMode_e::kPwm6xModeLimit;
    regMap.ctrl05.ovp_en = drv8316cr::Ctrl05::Ovp_e::kOvpEn;
    regMap.ctrl06.ocp_mode = drv8316cr::Ctrl06::OcpMode_e::kOcpModeAutoRetry;
    regMap.ctrl07.csa_gain = drv8316cr::Ctrl07::CsaGain_e::k0p6;
    regMap.ctrl08.buck_sel = drv8316cr::Ctrl08::BuckSel_e::kBuckSel4p7V;
    regMap.ctrl0c.dly_target = drv8316cr::Ctrl0C::DelayTarget_e::k1p6us;

    drv8316cr::CtrlRegs_t ctrlRegs{};
    drv8316cr::buildCtrlRegs(regMap, ctrlRegs);

    UNSIGNED_LONGS_EQUAL(0x03u, ctrlRegs.ctrl03);
    UNSIGNED_LONGS_EQUAL(0x02u, ctrlRegs.ctrl04);
    UNSIGNED_LONGS_EQUAL(0x04u, ctrlRegs.ctrl05);
    UNSIGNED_LONGS_EQUAL(0x02u, ctrlRegs.ctrl06);
    UNSIGNED_LONGS_EQUAL(0x02u, ctrlRegs.ctrl07);
    UNSIGNED_LONGS_EQUAL(0x04u, ctrlRegs.ctrl08);
    UNSIGNED_LONGS_EQUAL(0x07u, ctrlRegs.ctrl0C);

    drv8316cr::RegisterMap decoded{};
    drv8316cr::readRegisterMap(ctrlRegs, decoded);

    CHECK_TRUE(decoded.ctrl03.reg_lock == drv8316cr::Ctrl03::RegLock_e::kUnlockAll);
    CHECK_TRUE(decoded.ctrl04.pwm_mode == drv8316cr::Ctrl04::PwmMode_e::kPwm6xModeLimit);
    CHECK_TRUE(decoded.ctrl05.ovp_en == drv8316cr::Ctrl05::Ovp_e::kOvpEn);
    CHECK_TRUE(decoded.ctrl06.ocp_mode == drv8316cr::Ctrl06::OcpMode_e::kOcpModeAutoRetry);
    CHECK_TRUE(decoded.ctrl07.csa_gain == drv8316cr::Ctrl07::CsaGain_e::k0p6);
    CHECK_TRUE(decoded.ctrl08.buck_sel == drv8316cr::Ctrl08::BuckSel_e::kBuckSel4p7V);
    CHECK_TRUE(decoded.ctrl0c.dly_target == drv8316cr::Ctrl0C::DelayTarget_e::k1p6us);
}
