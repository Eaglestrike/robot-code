#pragma once

#include "../action.h"

#include <frc/kinematics/DifferentialDriveWheelSpeeds.h>
#include <frc/trajectory/Trajectory.h>
#include <frc2/Timer.h>

#include <subsystems/drive.h>

namespace team114 {
namespace c2020 {
namespace auton {

class DrivePathAction : public Action {
   public:
    DrivePathAction(frc::Trajectory&& traj) : traj_(std::move(traj)) {}
    virtual void Start() override {
        Drive::GetInstance().SetWantDriveTraj(std::move(traj_));
    }
    virtual void Periodic() override {}
    virtual bool Finished() override {
        return Drive::GetInstance().FinishedTraj();
    }
    virtual void Stop() override {
        Drive::GetInstance().SetWantRawOpenLoop({0_mps, 0_mps});
    }

   private:
    frc::Trajectory traj_;
};

class DriveOpenLoopAction : public Action {
   public:
    DriveOpenLoopAction(frc::DifferentialDriveWheelSpeeds cmd,
                        units::second_t dur)
        : cmd_(cmd), dur_(dur) {}
    virtual void Start() override {
        timer_.Reset();
        Drive::GetInstance().SetWantRawOpenLoop(cmd_);
        timer_.Start();
    }
    virtual void Periodic() override {}
    virtual bool Finished() override { return timer_.Get() > dur_; }
    virtual void Stop() override {
        Drive::GetInstance().SetWantRawOpenLoop({0_mps, 0_mps});
        timer_.Stop();
    }

   private:
    frc::DifferentialDriveWheelSpeeds cmd_;
    units::second_t dur_;
    frc2::Timer timer_;
};

}  // namespace auton
}  // namespace c2020
}  // namespace team114
