#include "climber.h"

#include <iostream>

namespace team114 {
namespace c2020 {
    
/** calls other constructor **/
Climber::Climber() : Climber(conf::GetConfig().climber) {}
    
/** 
* Sets configuration of components (latch, brake, action, etc.)
* Defines values for limits, forward/reverse output, etc.
* Sets SoftLimitEnable as true
**/
Climber::Climber(const conf::ClimberConfig& cfg)
    : cfg_{cfg},
      master_talon_{cfg_.master_id},
      slave_talon_{cfg_.slave_id},
      latch_{frc::Solenoid{cfg_.release_solenoid_id}},
      brake_{frc::Solenoid{cfg_.brake_solenoid_id}},
      action_{CurrentAction::Climbing} {
    TalonSRXConfiguration c;
    c.peakCurrentLimit = cfg_.current_limit;
    c.peakCurrentDuration = 10;
    c.continuousCurrentLimit = 0.9 * cfg_.current_limit;
    c.primaryPID.selectedFeedbackSensor =
        FeedbackDevice::CTRE_MagEncoder_Relative;
    c.primaryPID.selectedFeedbackCoefficient = 1.0;
    c.openloopRamp = 0.2;
    c.peakOutputForward = 1.0;
    c.peakOutputReverse = -1.0;
    c.nominalOutputForward = 0.0;
    c.nominalOutputReverse = 0.0;
    c.voltageCompSaturation = 12.0;
    // TODO(josh) tune
    // Unfinished function
    c.forwardSoftLimitEnable = true;
    c.reverseSoftLimitEnable = true;
    c.forwardSoftLimitThreshold = cfg_.forward_soft_limit_ticks;
    c.reverseSoftLimitThreshold = -0.05 * cfg_.avg_ticks_per_inch;
    // TODO(josh) logs everywhere
    // Unfinished function
    for (int i = 0; i < 10; i++) {
        auto err = master_talon_.ConfigAllSettings(c, 100);
        if (err == ErrorCode::OKAY) {
            break;
        }
    }
    for (int i = 0; i < 10; i++) {
        auto err = slave_talon_.ConfigAllSettings(c, 100);
        if (err == ErrorCode::OKAY) {
            break;
        }
    }
// Voltage compensation enabled
// Current limit enabled
// Neutral mode is set on brake
    master_talon_.EnableVoltageCompensation(true);
    master_talon_.EnableCurrentLimit(true);
    master_talon_.SetSensorPhase(true);
    master_talon_.SelectProfileSlot(0, 0);
    master_talon_.SetNeutralMode(NeutralMode::Brake);
// Frame periods set (configured) for master and slave talons (Pid talon is master talon)
    conf::SetFramePeriodsForPidTalon(master_talon_);
    conf::SetFramePeriodsForSlaveTalon(slave_talon_);
    slave_talon_.Follow(master_talon_);
}
/** 
* Increments/updates position of the climber. If the climber is climbing, it moves to the selected position.
* If the climber is currently resetting, the latch is set to true.
* True means that latch is not set (counterintuitive).
**/
void Climber::Periodic() {
    if (action_ == Climber::CurrentAction::Climbing) {
        SDB_NUMERIC(double, CimberPosTicks)
        climb_pos = master_talon_.GetSelectedSensorPosition();
        climb_pos++;
        latch_.Set(master_talon_.GetSelectedSensorPosition() >
                   cfg_.initial_step_ticks);
    } else if (action_ == Climber::CurrentAction::Resetting) {
        latch_.Set(true);
    }
}
/**
* When the climber stops, the latch and brake are set to false.
* False means that brake and latch are set.
**/
void Climber::Stop() {
    brake_.Set(false);
    latch_.Set(false);
}

/** empty, presumable purpose to reset sensors **/
void Climber::ZeroSensors() {}
/** empty, presumable purpose to output data**/
void Climber::OutputTelemetry() {}

// TODO in disabledinit, setwantdirection(neutral)
/** Sets desired climber direction, either up, down, or neutral. **/
void Climber::SetWantDirection(Climber::Direction direction) {
    if (action_ != CurrentAction::Climbing) {
        master_talon_.OverrideSoftLimitsEnable(true);
        master_talon_.SetSelectedSensorPosition(0);
    }
    action_ = CurrentAction::Climbing;
    switch (direction) {
         // Clibmer set in upwards direction, brake set to true
        case Climber::Direction::Up:
            brake_.Set(true);
            master_talon_.Set(ControlMode::PercentOutput, cfg_.ascend_command);
            break;
         // When climber moves down, brake is set to true
        case Climber::Direction::Down:
            brake_.Set(true);
            master_talon_.Set(ControlMode::PercentOutput, cfg_.descend_command);
            break;
        // Climber set in neutral direction, brake set to false
        case Climber::Direction::Neutral:
            brake_.Set(false);
            master_talon_.Set(ControlMode::PercentOutput, 0.0);
            break;
    }
}
/** Happens when resetting climber. Disables break? Not sure what wind is (is there something being wound?) **/
void Climber::SetZeroingWind(Climber::Direction direction) {
    if (action_ != CurrentAction::Resetting) {
        master_talon_.OverrideSoftLimitsEnable(false);
    }
    
    action_ = CurrentAction::Resetting;
    switch (direction) {
       // brake is set to true (not enabled)
       // If the climber is currently going 
        case Climber::Direction::Up:
            brake_.Set(true);
            master_talon_.Set(ControlMode::PercentOutput,
                              0.75 * cfg_.ascend_command);
            break;
        
        case Climber::Direction::Down:
            brake_.Set(true);
            master_talon_.Set(ControlMode::PercentOutput,
                              0.75 * cfg_.descend_command);
            break;
        // Climber set in neutral direction, brake set to true
        case Climber::Direction::Neutral:
            brake_.Set(true);
            master_talon_.Set(ControlMode::PercentOutput, 0.0);
            break;
    }
}

}  // namespace c2020
}  // namespace team114
