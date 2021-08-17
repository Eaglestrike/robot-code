#pragma once

#include <frc/DigitalInput.h>
#include "shims/minimal_phoenix.h"

#include "config.h"
#include "subsystem.h"
#include "subsystems/hood.h"
#include "subsystems/intake.h"
#include "subsystems/limelight.h"
#include "subsystems/auto_shoot.h"
#include "util/sdb_types.h"

namespace team114 {
namespace c2020 {

class BallPath : public Subsystem {
    SUBSYSTEM_PRELUDE(BallPath)
   public:
    BallPath(const conf::RobotConfig& cfg);

    void Periodic() override;
    void Stop() override;
    void ZeroSensors() override;
    void OutputTelemetry() override;

    enum State {
        Idle,
        Intk,
        Unjm,
        Shoot,
    };
    void SetWantState(State state);
    enum ShotType {
        Short,
        Med,
        Long,
    };
    void SetShot();
    void ShortShot();
    void LongShot();

   private:
    enum class Direction {
        Forward,
        Reverse,
        Neutral,
    };
    void SetChannelDirection(Direction dir);
    void SetSerializerDirection(Direction dir);
    struct Shot {
        double hood_angle;
        double flywheel_sp;
        double kicker_cmd;
    };
    void SetWantShot(Shot shot);
    void UpdateShotFromVision();
    bool ReadyToShoot();

    const conf::ShooterConifg shooter_cfg_;
    const conf::BallChannelConfig channel_cfg_;
    TalonFX shooter_master_;
    TalonFX shooter_slave;
    TalonSRX kicker_;
    TalonSRX serializer_;
    TalonSRX channel_;
    frc::DigitalInput s0_;
    frc::DigitalInput s1_;
    frc::DigitalInput s2_;
    frc::DigitalInput s3_;

    State state_;
    Shot current_shot_;

    Intake& intake_;
    Limelight& limelight_;
    Hood& hood_;

    
};

}  // namespace c2020
}  // namespace team114
