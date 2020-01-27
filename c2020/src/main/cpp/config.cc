#include "config.h"

#include <cmath>
#include <fstream>
#include <optional>
#include <string>

namespace team114 {
namespace c2020 {

// WPILib chose GCC 7, so no C++20 designated initializers here

// sim / testing before we have a real robot
const RobotConfig MakeDefaultRobotConfig() {
    RobotConfig c;
    c.mac_address = "aa:bb:cc:dd:ee:ff";
    c.drive.left_master_id = 21;
    c.drive.left_slave_id = 22;
    c.drive.right_master_id = 23;
    c.drive.right_slave_id = 24;
    c.drive.track_width = 0.661924_m;
    c.drive.meters_per_falcon_tick =
        1.0 / 2048.0 * 10.0 / 62.0 * 18.0 / 30.0 * 6 * M_PI * 0.0254_m;
    return c;
}

static std::optional<RobotConfig> CONFIG;

RobotConfig& GetConfig() {
    if (CONFIG.has_value()) {
        return CONFIG.value();
    }
    // TODO(josh): log the chosen config
    std::ifstream ifs("/sys/class/net/eth0/address");
    if (!ifs.fail() || !ifs.is_open()) {
        CONFIG = MakeDefaultRobotConfig();
        return GetConfig();
    }
    std::string rio_mac((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    // TODO(josh) fill in with real RIOs
    // TODO(josh) check for newline in file read
    RobotConfig config_a = MakeDefaultRobotConfig();
    RobotConfig config_b = MakeDefaultRobotConfig();
    if (config_a.mac_address == rio_mac) {
        CONFIG = std::move(config_a);
    } else if (config_a.mac_address == rio_mac) {
        CONFIG = std::move(config_b);
    } else {
        CONFIG = MakeDefaultRobotConfig();
    }
    return GetConfig();
}

void DriveFalconCommonConfig(TalonFX& falcon) {
    TalonFXConfiguration c;
    // ====== Talon FX CFG
    c.supplyCurrLimit =
        SupplyCurrentLimitConfiguration{true, 50.0, 55.0, 0.200};
    // StatorCurrentLimitConfiguration statorCurrLimit

    // TODO(josh) enable FOC on arrival
    c.motorCommutation = MotorCommutation::Trapezoidal;
    // absoluteSensorRange = AbsoluteSensorRange::Unsigned_0_to_360 double
    // integratedSensorOffsetDegrees = 0
    c.initializationStrategy = SensorInitializationStrategy::BootToZero;
    // ===== Base Talon CFG
    c.primaryPID.selectedFeedbackSensor = FeedbackDevice::IntegratedSensor;
    c.primaryPID.selectedFeedbackCoefficient = 1.0;
    // BaseTalonPIDSetConfiguration auxiliaryPID
    // LimitSwitchSource forwardLimitSwitchSource
    // LimitSwitchSource reverseLimitSwitchSource
    // int forwardLimitSwitchDeviceID
    // int reverseLimitSwitchDeviceID
    // LimitSwitchNormal forwardLimitSwitchNormal
    // LimitSwitchNormal reverseLimitSwitchNormal
    // FeedbackDevice sum0Term
    // FeedbackDevice sum1Term
    // FeedbackDevice diff0Term
    // FeedbackDevice diff1Term
    // ======= BaseMotor Param cfg
    c.openloopRamp = 0.2;    // time from neutral to full
    c.closedloopRamp = 0.0;  // disable ramping, should be running a profile
    c.peakOutputForward = 1.0;
    c.peakOutputReverse = -1.0;
    c.nominalOutputForward = 0.0;
    c.nominalOutputReverse = 0.0;
    // double neutralDeadband
    c.voltageCompSaturation = 12.0;
    // int voltageMeasurementFilter
    // TODO(josh) tune these
    // https://phoenix-documentation.readthedocs.io/en/latest/ch14_MCSensor.html#velocity-measurement-filter
    c.velocityMeasurementPeriod = VelocityMeasPeriod::Period_5Ms;
    c.velocityMeasurementWindow = 2;
    // int forwardSoftLimitThreshold
    // int reverseSoftLimitThreshold
    c.forwardSoftLimitEnable = false;
    c.reverseSoftLimitEnable = false;
    // TODO(Josh) tune for Ramsete
    c.slot0.allowableClosedloopError = 0;
    c.slot0.closedLoopPeakOutput = 1.0;
    c.slot0.closedLoopPeriod = 1;
    // TODO(josh) tune
    // c.slot0.integralZone =
    // c.slot0.maxIntegralAccumulator =
    // https://phoenix-documentation.readthedocs.io/en/latest/ch16_ClosedLoop.html#calculating-velocity-feed-forward-gain-kf
    c.slot0.kF = 0.0;
    c.slot0.kP = 0.0;
    c.slot0.kI = 0.0;
    c.slot0.kD = 0.0;
    // SlotConfiguration slot1
    // SlotConfiguration slot2
    // SlotConfiguration slot3
    // bool auxPIDPolarity
    // FilterConfiguration remoteFilter0
    // FilterConfiguration remoteFilter1
    // int motionCruiseVelocity
    // int motionAcceleration
    // int motionCurveStrength
    // int motionProfileTrajectoryPeriod
    // bool feedbackNotContinuous
    // bool remoteSensorClosedLoopDisableNeutralOnLOS
    // bool clearPositionOnLimitF
    // bool clearPositionOnLimitR
    // bool clearPositionOnQuadIdx
    // bool limitSwitchDisableNeutralOnLOS
    // bool softLimitDisableNeutralOnLOS
    // int pulseWidthPeriod_EdgesPerRot
    // int pulseWidthPeriod_FilterWindowSz
    // bool trajectoryInterpolationEnable
    // ======= Custom Param CFG
    // int customParam0
    // int customParam1
    // bool enableOptimizations
    // TODO(josh) logs everywhere
    for (int i = 0; i < 10; i++) {
        auto err = falcon.ConfigAllSettings(c, 100);
        if (err == ErrorCode::OKAY) {
            break;
        }
    }
    falcon.EnableVoltageCompensation(true);
    // TODO(josh) falcon's dont have enable current limit?
    falcon.SelectProfileSlot(0, 0);
    falcon.SetNeutralMode(NeutralMode::Brake);
}

// CAN metrics docs
// https://phoenix-documentation.readthedocs.io/en/latest/ch18_CommonAPI.html#can-bus-utilization-error-metrics
inline static ErrorCode SetDriveCommonFramePeriods(TalonFX& falcon) {
    constexpr int lng = 200;
    ErrorCollection err;
    err.NewError(falcon.SetStatusFramePeriod(
        StatusFrameEnhanced::Status_3_Quadrature, lng));
    err.NewError(falcon.SetStatusFramePeriod(
        StatusFrameEnhanced::Status_4_AinTempVbat, lng));
    err.NewError(falcon.SetStatusFramePeriod(
        StatusFrameEnhanced::Status_8_PulseWidth, lng));
    err.NewError(falcon.SetStatusFramePeriod(
        StatusFrameEnhanced::Status_10_MotionMagic, lng));
    err.NewError(falcon.SetStatusFramePeriod(
        StatusFrameEnhanced::Status_12_Feedback1, lng));
    err.NewError(falcon.SetStatusFramePeriod(
        StatusFrameEnhanced::Status_14_Turn_PIDF1, lng));
    return err.GetFirstNonZeroError();
}
constexpr int kStatusFrameAttempts = 3;

// TODO(josh) log here
void SetDriveMasterFramePeriods(TalonFX& falcon) {
    for (int i = 0; i < kStatusFrameAttempts; i++) {
        ErrorCollection err;
        err.NewError(SetDriveCommonFramePeriods(falcon));
        err.NewError(falcon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_1_General, 5));
        err.NewError(falcon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_2_Feedback0, 5));
        err.NewError(falcon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_13_Base_PIDF0, 50));
        if (err.GetFirstNonZeroError() == ErrorCode::OK) {
            return;
        }
    }
}

void SetDriveSlaveFramePeriods(TalonFX& falcon) {
    for (int i = 0; i < kStatusFrameAttempts; i++) {
        ErrorCollection err;
        err.NewError(SetDriveCommonFramePeriods(falcon));
        err.NewError(falcon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_1_General, 255));
        err.NewError(falcon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_2_Feedback0, 255));
        err.NewError(falcon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_13_Base_PIDF0, 200));
        if (err.GetFirstNonZeroError() == ErrorCode::OK) {
            return;
        }
    }
}

}  // namespace c2020
}  // namespace team114
