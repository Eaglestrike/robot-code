#include "config.h"

#include <cmath>
#include <fstream>
#include <optional>
#include <string>

#include "util/number_util.h"

namespace team114 {
namespace c2020 {
namespace conf {

// WPILib chose GCC 7, so no C++20 designated initializers here

/**
* This function is declaring all the constants that will be needed for the robot through obj c
* Includes PID constants, ticks, etc
**/
const RobotConfig MakeDefaultRobotConfig() {
    RobotConfig c;
    // /sys/class/net/<if>/address is newline terminated, match here
    c.mac_address = "aa:bb:cc:dd:ee:ff\n";

    c.drive.left_master_id = 21;
    c.drive.left_slave_id = 22;
    c.drive.right_master_id = 23;
    c.drive.right_slave_id = 24;
    c.drive.track_width = 0.661924_m;
    c.drive.meters_per_falcon_tick =
        1.0 / 2048.0 * 10.0 / 62.0 * 18.0 / 30.0 * 6 * M_PI * 0.0254_m;
    c.drive.traj_max_vel = 2.5_mps;
    c.drive.traj_max_accel = units::meters_per_second_squared_t{5.0};
    // currently this is g with 2x FOS
    c.drive.traj_max_centrip_accel = units::meters_per_second_squared_t{4.9};
    c.drive.orient_kp = 0.45;
    c.drive.orient_ki = 0.0;
    c.drive.orient_kd = 0.0;
    c.drive.orient_vel = 0.0;
    c.drive.orient_acc = 0.0;

    c.ctrl_panel.talon_id = 31;
    c.ctrl_panel.current_limit = 22;
    c.ctrl_panel.kP = 1.5;
    c.ctrl_panel.kI = 0.0;
    c.ctrl_panel.kD = 5.0;
    c.ctrl_panel.ticks_per_inch = 4096.0 / (3.5 * M_PI);
    c.ctrl_panel.ticks_per_color_slice =
        12.56 * 0.96 * c.ctrl_panel.ticks_per_inch;
    c.ctrl_panel.rot_control_ticks =
        32 * M_PI * 3.5 * c.ctrl_panel.ticks_per_inch;
    c.ctrl_panel.scoot_cmd = 0.5;
    c.ctrl_panel.solenoid_channel = 4;

    // TODO the rest
    c.intake.rot_talon_id = 41;
    c.intake.roller_talon_id = 42;
    c.intake.intake_cmd = 0.60;
    c.intake.rot_current_limit = 10;
    c.intake.zeroing_kp = 0.001;
    c.intake.zeroing_vel = 8;
    c.intake.roller_current_limit = 20;
    c.intake.abs_enc_tick_offset = 2945;
    c.intake.abs_ticks_per_rot = 4096.0 * 36.0 / 22.0;
    c.intake.rel_ticks_per_abs_tick = 1.0;
    c.intake.rads_per_rel_tick =
        2 * M_PI / c.intake.abs_ticks_per_rot / c.intake.rel_ticks_per_abs_tick;
    c.intake.zeroed_rad_from_vertical = 15.4 * M_PI / 180.0;
    c.intake.SinekF = -0.063;
    c.intake.kP = 1.0;
    c.intake.kI = 0.005;
    c.intake.kD = 0.0;
    c.intake.profile_acc = 3000;
    c.intake.profile_vel = 2000;
    c.intake.stowed_pos_ticks = 1222;
    c.intake.intaking_pos_ticks = 2350;
    // c.intake.trench_driving_pos_ticks = 20.0 / 36.0 * 4096.0;

    c.hood.talon_id = 52;
    c.hood.ticks_per_degree = 4096.0 * 350.0 / 28.0 / 360.0;
    c.hood.min_degrees = 20;  // TODO
    c.hood.max_degrees = 64;
    c.hood.current_limit = 20;
    c.hood.zeroing_current = 6;
    c.hood.zeroing_kp = .001;
    c.hood.zeroing_vel = 15;
    c.hood.profile_acc = 20000;  // TODO
    c.hood.profile_vel = 9000;   // TODO
    c.hood.kP = 2.0;
    c.hood.kI = 0.02;
    c.hood.kD = 10.0;
    c.hood.ctre_curve_smoothing = 2;

    c.shooter.master_id = 25; //left side of shooter
    c.shooter.slave_id = 26; //right side of shooter
    c.shooter.shooter_current_limit = 40;
    c.shooter.kF = 0.0160;
    c.shooter.kP = 0.25;
    c.shooter.kP = 0.0;
    c.shooter.kI = 0.0;
    c.shooter.kD = 0.0;
    c.shooter.meas_period = VelocityMeasPeriod::Period_2Ms;
    c.shooter.meas_filter_width = 32;
    c.shooter.shootable_err_pct = 0.04;
    c.shooter.kicker_id = 51;
    c.shooter.kicker_current_limit = 20;
    c.shooter.kicker_cmd = 0.45;

    c.ball_channel.serializer_id = 43;
    c.ball_channel.channel_id = 44;
    c.ball_channel.current_limit = 25;
    c.ball_channel.serializer_cmd = 1.00;
    c.ball_channel.channel_cmd = 0.75;
    c.ball_channel.s0_port = 3;
    c.ball_channel.s1_port = 0;
    c.ball_channel.s2_port = 2;
    c.ball_channel.s3_port = 1;

    c.climber.master_id = 19;
    c.climber.slave_id = 20;
    c.climber.current_limit = 35;
    c.climber.release_solenoid_id = 7;
    c.climber.brake_solenoid_id = 6;
    c.climber.avg_ticks_per_inch = 4096.0 * 9.0 / ((.969 + 1.25) * M_PI);
    c.climber.initial_step_ticks = 10.0 * c.climber.avg_ticks_per_inch;
    c.climber.forward_soft_limit_ticks = 122000;
    c.climber.ascend_command = 1.00;
    c.climber.descend_command = -1.00;

    c.limelight.name = "limelight";
    c.limelight.table_name = "limelight";
    c.limelight.diff_height = 2.496_m - 0.55_m;
    c.limelight.angle_above_horizontal = DegToRad(8.0);

    return c;
}

/**
* Basic getter, returning the configuration to uphold encapsulation in oop
**/
RobotConfig& GetConfig() {
    static std::optional<RobotConfig> CONFIG;
    if (CONFIG.has_value()) {
        return CONFIG.value();
    }
    // TODO(josh): log the chosen config
    std::ifstream ifs("/sys/class/net/eth0/address");
    if (!ifs.good() || !ifs.is_open()) {
        CONFIG = MakeDefaultRobotConfig();
        return GetConfig();
    }
    std::string rio_mac((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    // TODO(josh) fill in with real RIOs
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

/**
* Defining constants for Talon configurations
* This includes current limit, nominal outputs, etc regarding talon motors
*/
void DriveFalconCommonConfig(TalonFX& falcon) {
    TalonFXConfiguration c;
    // ====== Talon FX CFG
    c.supplyCurrLimit =
        SupplyCurrentLimitConfiguration{true, 50.0, 55.0, 0.100};
    // StatorCurrentLimitConfiguration statorCurrLimit

    // TODO(josh) enable FOC on arrival
    c.motorCommutation = MotorCommutation::Trapezoidal;
    // absoluteSensorRange = AbsoluteSensorRange::Unsigned_0_to_360 double
    // integratedSensorOffsetDegrees = 0
    c.initializationStrategy = SensorInitializationStrategy::BootToZero;
    // ===== Base Talon CFG
    c.primaryPID.selectedFeedbackSensor = FeedbackDevice::IntegratedSensor;
    c.primaryPID.selectedFeedbackCoefficient = 1.0;
    // ======= BaseMotor Param cfg
    c.openloopRamp = 0.25;   // time from neutral to full
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

//Basically finding and storing multiple error codes for Talonfx
//Explains why you need error handling
//https:github.com/CrossTheRoadElec/Phoenix-Documentation/blob/master/Legacy/README.md#error-handling
inline static ErrorCode SetDriveCommonFramePeriods(TalonFX& falcon) {
    constexpr int lng = 255;
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
//Similiar to the function above, it just adds error to collection
//Main difference is that you don't return it. Also this is master meaning it controls slave
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

/** 
* Collects error codes, however master controls 
**/ 
void SetDriveSlaveFramePeriods(TalonFX& falcon) {
    for (int i = 0; i < kStatusFrameAttempts; i++) {
        ErrorCollection err;
        err.NewError(SetDriveCommonFramePeriods(falcon));
        err.NewError(falcon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_1_General, 255));
        err.NewError(falcon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_2_Feedback0, 255));
        err.NewError(falcon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_13_Base_PIDF0, 255));
        if (err.GetFirstNonZeroError() == ErrorCode::OK) {
            return;
        }
    }
}

/**
* Setting values and configurations for Talons
* Also collecting error from functions
**/ 
void SetFramePeriodsForPidTalon(TalonSRX& talon, FeedbackType feedback_type) {
    constexpr int lng = 255;
    constexpr int shrt = 10;
    int quad = lng;
    int pulsewidth = lng;
    switch (feedback_type) {
        case FeedbackType::Both:
            quad = shrt;
            pulsewidth = shrt;
            break;
        case FeedbackType::Quadrature:
            quad = shrt;
            break;
        case FeedbackType::PulseWidth:
            pulsewidth = shrt;
            break;
        case FeedbackType::None:
            break;
    }
    for (int i = 0; i < kStatusFrameAttempts; i++) {
        ErrorCollection err;
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_1_General, shrt));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_2_Feedback0, shrt));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_3_Quadrature, quad));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_4_AinTempVbat, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_8_PulseWidth, pulsewidth));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_10_MotionMagic, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_12_Feedback1, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_14_Turn_PIDF1, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_13_Base_PIDF0, 50));
        if (err.GetFirstNonZeroError() == ErrorCode::OK) {
            return;
        }
    }
}

void SetFramePeriodsForPidTalonFX(TalonFX& talon, FeedbackType feedback_type) {
    constexpr int lng = 255;
    constexpr int shrt = 10;
    int quad = lng;
    int pulsewidth = lng;
    switch (feedback_type) {
        case FeedbackType::Both:
            quad = shrt;
            pulsewidth = shrt;
            break;
        case FeedbackType::Quadrature:
            quad = shrt;
            break;
        case FeedbackType::PulseWidth:
            pulsewidth = shrt;
            break;
        case FeedbackType::None:
            break;
    }
    for (int i = 0; i < kStatusFrameAttempts; i++) {
        ErrorCollection err;
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_1_General, shrt));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_2_Feedback0, shrt));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_3_Quadrature, quad));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_4_AinTempVbat, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_8_PulseWidth, pulsewidth));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_10_MotionMagic, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_12_Feedback1, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_14_Turn_PIDF1, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_13_Base_PIDF0, 50));
        if (err.GetFirstNonZeroError() == ErrorCode::OK) {
            return;
        }
    }
}


/**
* Error collecting for  Open loop TalonSRX's
**/ 
void SetFramePeriodsForOpenLoopTalon(TalonSRX& talon) {
    constexpr int lng = 255;
    constexpr int shrt = 20;
    for (int i = 0; i < kStatusFrameAttempts; i++) {
        ErrorCollection err;
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_1_General, shrt));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_2_Feedback0, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_3_Quadrature, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_4_AinTempVbat, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_8_PulseWidth, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_10_MotionMagic, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_12_Feedback1, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_14_Turn_PIDF1, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_13_Base_PIDF0, lng));
        if (err.GetFirstNonZeroError() == ErrorCode::OK) {
            return;
        }
    }
}

/**
* Another Slave function which means controlled by master
* collect error
**/ 
void SetFramePeriodsForSlaveTalon(TalonSRX& talon) {
    constexpr int lng = 255;
    for (int i = 0; i < kStatusFrameAttempts; i++) {
        ErrorCollection err;
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_1_General, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_2_Feedback0, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_3_Quadrature, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_4_AinTempVbat, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_8_PulseWidth, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_10_MotionMagic, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_12_Feedback1, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_14_Turn_PIDF1, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_13_Base_PIDF0, lng));
        if (err.GetFirstNonZeroError() == ErrorCode::OK) {
            return;
        }
    }
}

void SetFramePeriodsForSlaveTalonFX(TalonFX& talon) {
    constexpr int lng = 255;
    for (int i = 0; i < kStatusFrameAttempts; i++) {
        ErrorCollection err;
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_1_General, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_2_Feedback0, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_3_Quadrature, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_4_AinTempVbat, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_8_PulseWidth, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_10_MotionMagic, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_12_Feedback1, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_14_Turn_PIDF1, lng));
        err.NewError(talon.SetStatusFramePeriod(
            StatusFrameEnhanced::Status_13_Base_PIDF0, lng));
        if (err.GetFirstNonZeroError() == ErrorCode::OK) {
            return;
        }
    }
}

}  // namespace conf
}  // namespace c2020
}  // namespace team114
