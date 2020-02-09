#pragma once

#include <memory>

#include "action.h"

namespace team114 {
namespace c2020 {
namespace auton {

class AutoExecutor {
   public:
    AutoExecutor(std::unique_ptr<Action>&& action)
        : action_{std::move(action)} {}

    ~AutoExecutor() { Stop(); }

    AutoExecutor& operator=(AutoExecutor&&) = default;

    void Periodic() {
        switch (state_) {
            case RunState::UNINIT:
                action_->Start();
                state_ = RunState::RUNNING;
                __attribute__((fallthrough));
            case RunState::RUNNING:
                if (action_->Finished()) {
                    action_->Stop();
                    state_ = RunState::FINISHED;
                } else {
                    action_->Periodic();
                }
                break;
            case RunState::FINISHED:
                break;
            default:
                break;
        }
    }

    bool Finished() { return state_ == RunState::FINISHED; }

    void Stop() {
        // ensure we don't Stop() the action twice or if it hasn't started
        if (state_ == RunState::FINISHED || state_ == RunState::UNINIT) {
            return;
        }
        action_->Stop();
        state_ = RunState::FINISHED;
    }

   private:
    enum class RunState { UNINIT, RUNNING, FINISHED };
    RunState state_{RunState::UNINIT};
    std::unique_ptr<Action> action_;
};

}  // namespace auton
}  // namespace c2020
}  // namespace team114
