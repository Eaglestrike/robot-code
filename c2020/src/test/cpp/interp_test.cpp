#include "util/interp_map.h"

#include "gtest/gtest.h"

#include <units/units.h>

using namespace team114::c2020;

TEST(InterpMap, DoubleDouble) {
    InterpolatingMap<double, double, ArithmeticInverseInterp<double>,
                     ArithmeticInterp<double>>
        map{2};

    map[0.0] = 10.0;
    map[5.0] = 20.0;

    EXPECT_NEAR(map.InterpAt(2.5).second, 15.0, 0.001);
    EXPECT_NEAR(map.InterpAt(1.25).second, 12.5, 0.001);
    EXPECT_NEAR(map.InterpAt(3.75).second, 17.5, 0.001);

    EXPECT_NEAR(map.InterpAt(-0.1).second, 10.0, 0.001);
    EXPECT_NEAR(map.InterpAt(5.1).second, 20.0, 0.001);

    // insert one more, should remove the lowest elem
    map[10.0] = -10.0;
    EXPECT_NEAR(map.InterpAt(4.9).second, 20.0, 0.001);
    EXPECT_NEAR(map.InterpAt(8.33333).second, 0.0, 0.001);
}

static bool cmp(units::meter_t one, units::meter_t two, units::meter_t diff) {
    return units::math::abs(one - two) < diff;
}

TEST(InterpMap, DoubleMeters) {
    InterpolatingMap<double, units::meter_t, ArithmeticInverseInterp<double>,
                     ArithmeticInterp<units::meter_t>>
        map{2};

    map[0.0] = 10.0_m;
    map[5.0] = 20.0_m;

    EXPECT_TRUE(cmp(map.InterpAt(2.5).second, 15.0_m, 0.001_m));
    EXPECT_TRUE(cmp(map.InterpAt(1.25).second, 12.5_m, 0.001_m));
    EXPECT_TRUE(cmp(map.InterpAt(3.75).second, 17.5_m, 0.001_m));

    EXPECT_TRUE(cmp(map.InterpAt(-0.1).second, 10.0_m, 0.001_m));
    EXPECT_TRUE(cmp(map.InterpAt(5.1).second, 20.0_m, 0.001_m));

    // insert one more, should remove the lowest elem
    map[10.0] = -10.0_m;
    EXPECT_TRUE(cmp(map.InterpAt(4.9).second, 20.0_m, 0.001_m));
    EXPECT_TRUE(cmp(map.InterpAt(8.33333).second, 0.0_m, 0.001_m));
}

using namespace frc;
static bool cmp(Pose2d one, Pose2d two) {
    return units::math::abs(one.Rotation().Radians() -
                            two.Rotation().Radians()) < 0.01_rad &&
           units::math::abs(one.Translation().X() - two.Translation().X()) <
               0.01_m &&
           units::math::abs(one.Translation().Y() - two.Translation().Y()) <
               0.01_m;
}

TEST(InterpMap, DoublePose2d) {
    InterpolatingMap<double, Pose2d, ArithmeticInverseInterp<double>,
                     Pose2dInterp>
        map{100};
    // basic single unit stuff
    map[0.0] = {{5.0_m, -10.0_m}, 0.5_rad};
    map[100.0] = {{6.0_m, -10.0_m}, 0.5_rad};
    map[200.0] = {{6.0_m, -11.0_m}, 0.5_rad};
    map[300.0] = {{6.0_m, -11.0_m}, 1.0_rad};
    // arc driving:
    map[400.0] = {{1.0_m, 0.0_m}, 1.5708_rad};
    map[490.0] = {{0.0_m, 1.0_m}, 3.1416_rad};

    EXPECT_TRUE(cmp(map.InterpAt(50.0).second, {{5.5_m, -10.0_m}, 0.5_rad}));
    EXPECT_TRUE(cmp(map.InterpAt(150.0).second, {{6.0_m, -10.5_m}, 0.5_rad}));
    EXPECT_TRUE(cmp(map.InterpAt(250.0).second, {{6.0_m, -11.0_m}, 0.75_rad}));

    EXPECT_TRUE(
        cmp(map.InterpAt(430.0).second, {{0.866_m, 0.5_m}, 2.0944_rad}));
    EXPECT_TRUE(cmp(map.InterpAt(445.0).second,
                    {{0.707_m, 0.707_m}, 2.35619449019_rad}));
    EXPECT_TRUE(
        cmp(map.InterpAt(460.0).second, {{0.5_m, 0.866_m}, 2.6180_rad}));
}
