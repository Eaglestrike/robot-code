#include "control_panel.h"

#include <frc/DriverStation.h>

#include <iostream>

namespace team114 {
namespace c2020 {
/**
*   Constructor for control panel gets config from 
*/
ControlPanel::ControlPanel() : ControlPanel{conf::GetConfig().ctrl_panel} {}
/**
*   Constructor for Control panel, takes a control panel config and sets up the control panel
*/
ControlPanel::ControlPanel(const conf::ControlPanelConfig& cfg)
    : cfg_{cfg},
      talon_{cfg.talon_id},
      deploy_{frc::Solenoid{cfg.solenoid_channel}} {
    TalonSRXConfiguration c;
    c.peakCurrentLimit = cfg_.current_limit;
    c.peakCurrentDuration = 30;
    c.continuousCurrentLimit = 0.8 * cfg_.current_limit;
    c.primaryPID.selectedFeedbackSensor =
        FeedbackDevice::CTRE_MagEncoder_Relative;
    c.primaryPID.selectedFeedbackCoefficient = 1.0;
    c.closedloopRamp = 0.0;  // disable ramping, should be running a profile
    c.peakOutputForward = 1.0;
    c.peakOutputReverse = -1.0;
    c.nominalOutputForward = 0.0;
    c.nominalOutputReverse = 0.0;
    c.voltageCompSaturation = 12.0;
    c.slot0.allowableClosedloopError = 0;
    c.slot0.closedLoopPeakOutput = 1.0;
    c.slot0.closedLoopPeriod = 1;
    // TODO(josh) tune
    c.slot0.integralZone = 0;
    c.slot0.maxIntegralAccumulator = 0;
    c.slot0.kF = 0.0;
    c.slot0.kP = cfg_.kP;
    c.slot0.kI = cfg_.kI;
    c.slot0.kD = cfg_.kD;
    // TODO(josh) logs everywhere
    for (int i = 0; i < 10; i++) {
        auto err = talon_.ConfigAllSettings(c, 100);
        if (err == ErrorCode::OKAY) {
            break;
        }
    }
    talon_.EnableVoltageCompensation(true);
    talon_.EnableCurrentLimit(true);
    talon_.SelectProfileSlot(0, 0);
    talon_.SetNeutralMode(NeutralMode::Brake);
    talon_.SetSensorPhase(true);
    conf::SetFramePeriodsForPidTalon(talon_);
}
/**
*   Nothing
*/
void ControlPanel::Periodic() {}
/**
*   It sets control panel talon to 0 percent output
*/
void ControlPanel::Stop() { talon_.Set(ControlMode::PercentOutput, 0.0); }
/**
*   Nothing
*/
void ControlPanel::ZeroSensors() {}
/**
*   Nothing
*/
void ControlPanel::OutputTelemetry() {}
/**
*   Sets deployed to deployed. 
*/
void ControlPanel::SetDeployed(bool deployed) { deploy_.Set(deployed); }
/**
*    It tells the tallon to move a number of ticks that were input. 
*/
void ControlPanel::DoRotationControl() { MoveTicks(cfg_.rot_control_ticks); }
/**
*   It takes the current color input then rotates the wheel until it is the correct color.   
*/
void ControlPanel::DoPositionControl(ObservedColor current) {
    std::string gameData;
    gameData = frc::DriverStation::GetInstance().GetGameSpecificMessage();
    if (gameData.length() <= 0) {
        return;
    }
    int desiredIndex;
    int observedIndex;
    switch (gameData[0]) {
        case 'G':
            desiredIndex = 0;
            break;
        case 'B':
            desiredIndex = 1;
            break;
        case 'Y':
            desiredIndex = 2;
            break;
        case 'R':
            desiredIndex = 3;
            break;
        default:
            desiredIndex = -1;
            break;
    }

    switch (current) {
        case ObservedColor::Green:
            observedIndex = 0;
            break;
        case ObservedColor::Blue:
            observedIndex = 1;
            break;
        case ObservedColor::Yellow:
            observedIndex = 2;
            break;
        case ObservedColor::Red:
            observedIndex = 3;
            break;
        default:
            observedIndex = -1;
    }
    if (desiredIndex == -1 || observedIndex == -1) {
        return;
    }
    // compute shortest distance
    int steps = desiredIndex - observedIndex;
    while (steps > 2) {
        steps -= 4;
    }
    while (steps < -2) {
        steps += 4;
    }
    MoveTicks(steps * cfg_.ticks_per_color_slice);
}
/**
*   It moves the control panel talon forwards or backwards by the scood amount.
*/
void ControlPanel::Scoot(ScootDir dir) {
    switch (dir) {
        case ControlPanel::ScootDir::Forward:
            talon_.Set(ControlMode::PercentOutput, cfg_.scoot_cmd);
            break;
        case ControlPanel::ScootDir::Reverse:
            talon_.Set(ControlMode::PercentOutput, -cfg_.scoot_cmd);
            break;
        case ControlPanel::ScootDir::Neutral:
            talon_.Set(ControlMode::PercentOutput, 0.0);
            break;
    }
}
/**
*     Tells the tallon to move the inputed number of ticks
*/
void ControlPanel::MoveTicks(int ticks) {
    talon_.Set(ControlMode::Position,
               talon_.GetSelectedSensorPosition() + ticks);
}

}  // namespace c2020
}  // namespace team114
