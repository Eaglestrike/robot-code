#pragma once

#include "../action.h"

#include <frc2/Timer.h>
#include "subsystems/ball_path.h"

namespace team114 {
namespace c2020 {
namespace auton {

class ShootAction : public Action {
   public:
    ShootAction(units::second_t dur) : dur_{dur} {}
    virtual void Start() override {
        timer_.Reset();
        BallPath::GetInstance().SetWantShot(BallPath::ShotType::Short);
        BallPath::GetInstance().SetWantState(BallPath::State::Shoot);
        timer_.Start();
    }
    virtual void Periodic() override {}
    virtual bool Finished() override { return timer_.Get() > dur_; }
    virtual void Stop() override {
        BallPath::GetInstance().SetWantState(BallPath::State::Idle);
        timer_.Stop();
    }

   private:
    units::second_t dur_;
    frc2::Timer timer_;
};

}  // namespace auton
}  // namespace c2020
}  // namespace team114
