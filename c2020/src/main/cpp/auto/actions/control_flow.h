#pragma once

#include <memory>
#include <vector>
#include <algorithm>

#include "../action.h"

namespace team114 {
namespace c2020 {
namespace auton {

class SeriesAction : public Action {
   public:
    SeriesAction(std::vector<std::unique_ptr<Action>>&& actions)
        : actions_{std::move(actions)} {}

    virtual void Start() override {
        // reverse vector so we execute back-to-front
        std::reverse(actions_.begin(), actions_.end());
        // now we execute back-to-front
        if (actions_.empty()) {
            return;
        }
        actions_.back()->Start();
    }
    virtual void Periodic() override {
        if (actions_.empty()) {
            return;
        }
        if (actions_.back()->Finished()) {
            actions_.back()->Stop();
            actions_.pop_back();
            if (actions_.empty()) {
                return;
            }
            actions_.back()->Start();
            return;
        }
        actions_.back()->Periodic();
    }
    virtual bool Finished() override { return actions_.empty(); }
    virtual void Stop() override {
        if (!actions_.empty()) {
            actions_.back()->Stop();
        }
    }

   private:
    std::vector<std::unique_ptr<Action>> actions_;
};

class ParallelAction : public Action {
   public:
    ParallelAction(std::vector<std::unique_ptr<Action>>&& actions) {
        actions_.reserve(actions.size());
        for (auto&& action : actions) {
            actions_.emplace_back(std::move(action), false);
        }
    }
    virtual void Start() override {
        for (auto& action : actions_) {
            action.first->Start();
        }
    }
    virtual void Periodic() override {
        for (auto& action : actions_) {
            if (action.second) {
                continue;
            }
            if (action.first->Finished()) {
                action.first->Stop();
                action.second = true;
                continue;
            }
            action.first->Periodic();
        }
    }
    virtual bool Finished() override {
        for (auto& action : actions_) {
            if (!action.second) {
                return false;
            }
        }
        return true;
    }
    virtual void Stop() override {
        for (auto& action : actions_) {
            if (!action.second) {
                action.first->Stop();
            }
        }
    }

   private:
    // bool represents whether the action has completed
    // which ensures we don't double Stop()
    std::vector<std::pair<std::unique_ptr<Action>, bool>> actions_;
};

}  // namespace auton
}  // namespace c2020
}  // namespace team114
