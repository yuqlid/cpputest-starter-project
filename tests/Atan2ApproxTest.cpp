#include "CppUTest/TestHarness.h"

#include <cmath>

#include "utils/atan2.hpp"

namespace {

template <typename T>
T pi()
{
    return static_cast<T>(3.14159265358979323846);
}

template <typename Atan2>
void checkCloseToStdAtan2(double tolerance)
{
    const double values[] = {-10.0, -3.0, -1.0, -0.25, 0.0, 0.25, 1.0, 3.0, 10.0};

    for (double y : values) {
        for (double x : values) {
            const double expected = std::atan2(y, x);
            const double actual = Atan2::eval(y, x);

            DOUBLES_EQUAL(expected, actual, tolerance);
        }
    }
}

template <typename Atan2>
void checkFloatCloseToStdAtan2(float tolerance)
{
    const float values[] = {
        -10.0f, -3.0f, -1.0f, -0.25f, 0.0f, 0.25f, 1.0f, 3.0f, 10.0f};

    for (float y : values) {
        for (float x : values) {
            const float expected = std::atan2(y, x);
            const float actual = Atan2::eval(y, x);

            DOUBLES_EQUAL(expected, actual, tolerance);
        }
    }
}

struct IdentityApprox {
    template <typename T>
    static T eval(T x)
    {
        return x;
    }
};

}  // namespace

TEST_GROUP(Atan2ApproxDedicated)
{
};

TEST(Atan2ApproxDedicated, HandlesAxisAlignedVectorsWithSignedAtan2Semantics)
{
    DOUBLES_EQUAL(0.0, Atan2Rajan9::eval(0.0, 1.0), 0.000001);
    DOUBLES_EQUAL(pi<double>(), Atan2Rajan9::eval(0.0, -1.0), 0.000001);
    DOUBLES_EQUAL(pi<double>() / 2.0, Atan2Rajan9::eval(1.0, 0.0), 0.000001);
    DOUBLES_EQUAL(-pi<double>() / 2.0, Atan2Rajan9::eval(-1.0, 0.0), 0.000001);
    DOUBLES_EQUAL(0.0, Atan2Rajan9::eval(0.0, 0.0), 0.000001);
}

TEST(Atan2ApproxDedicated, MapsAllQuadrantsCloseToStdAtan2)
{
    DOUBLES_EQUAL(std::atan2(1.0, 2.0), Atan2Rajan9::eval(1.0, 2.0), 0.01);
    DOUBLES_EQUAL(std::atan2(2.0, 1.0), Atan2Rajan9::eval(2.0, 1.0), 0.01);
    DOUBLES_EQUAL(std::atan2(1.0, -2.0), Atan2Rajan9::eval(1.0, -2.0), 0.01);
    DOUBLES_EQUAL(std::atan2(2.0, -1.0), Atan2Rajan9::eval(2.0, -1.0), 0.01);
    DOUBLES_EQUAL(std::atan2(-1.0, -2.0), Atan2Rajan9::eval(-1.0, -2.0), 0.01);
    DOUBLES_EQUAL(std::atan2(-2.0, -1.0), Atan2Rajan9::eval(-2.0, -1.0), 0.01);
    DOUBLES_EQUAL(std::atan2(-1.0, 2.0), Atan2Rajan9::eval(-1.0, 2.0), 0.01);
    DOUBLES_EQUAL(std::atan2(-2.0, 1.0), Atan2Rajan9::eval(-2.0, 1.0), 0.01);
}

TEST(Atan2ApproxDedicated, ApproximationPoliciesStayCloseAcrossRepresentativeGrid)
{
    checkCloseToStdAtan2<Atan2Rajan7>(0.02);
    checkCloseToStdAtan2<Atan2Rajan9>(0.02);
    checkCloseToStdAtan2<Atan2Hastings>(0.02);
}

TEST(Atan2ApproxDedicated, FloatApproximationPoliciesStayCloseAcrossRepresentativeGrid)
{
    // Float is the embedded target type, so use a slightly wider tolerance than
    // the double grid above to account for single-precision std::atan2 and math.
    checkFloatCloseToStdAtan2<Atan2Rajan7>(0.021f);
    checkFloatCloseToStdAtan2<Atan2Rajan9>(0.021f);
    checkFloatCloseToStdAtan2<Atan2Hastings>(0.021f);
}

TEST(Atan2ApproxDedicated, CommonMapperCanUseAnotherApproximationPolicy)
{
    DOUBLES_EQUAL(0.25, (atan2_common<double, IdentityApprox>(0.25, 1.0)), 0.000001);
    DOUBLES_EQUAL(pi<double>() / 2.0 - 0.25,
                  (atan2_common<double, IdentityApprox>(1.0, 0.25)), 0.000001);
}

TEST(Atan2ApproxDedicated, FloatCommonMapperCanUseAnotherApproximationPolicy)
{
    // This uses a wider tolerance than the double policy test because the
    // embedded path instantiates atan2_common with float.
    DOUBLES_EQUAL(0.25f, (atan2_common<float, IdentityApprox>(0.25f, 1.0f)), 0.000001f);
    DOUBLES_EQUAL(pi<float>() / 2.0f - 0.25f,
                  (atan2_common<float, IdentityApprox>(1.0f, 0.25f)), 0.000001f);
}
