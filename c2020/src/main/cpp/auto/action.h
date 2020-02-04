#pragma once

#include <memory>
#include <vector>

#include "../util/constructor_macros.h"

namespace team114 {
namespace c2020 {
namespace auton {

class Action {
   public:
    Action() = default;
    Action(Action&&) = default;
    DISALLOW_COPY_ASSIGN(Action)

    // Since we will be using Action* free-ing derived classes,
    // we need this or UB https://stackoverflow.com/a/461224
    virtual ~Action() {}

    // Run once when action first scheduled
    virtual void Start() {}

    // Iterate
    // This function is not guaranteed to run if Finished() returns true early
    virtual void Periodic() {}

    // Informs the executor whether the action has completed.
    // The action will be unscheduled once this returns true.
    virtual bool Finished() { return true; }

    // Cleanup when action is un-scheduled, either due to completion or
    // termination. Other methods must not be invoked after Stop() is called.
    // Stop() is guaranteed to be invoked shortly after Finished() returns true.
    virtual void Stop() {}
};

class EmptyAction : public Action {
   public:
    virtual void Start() override {}
    virtual void Periodic() override {}
    virtual bool Finished() override { return true; }
    virtual void Stop() override {}
};

using ActionList = std::vector<std::unique_ptr<Action>>;

template <typename T>
void __add_action_list(ActionList& v, T&& action) {
    v.emplace_back(std::move(action));
}

template <typename T1, typename... T2>
void __add_action_list(ActionList& v, T1&& action, T2&&... actions) {
    v.emplace_back(std::move(action));
    __add_action_list(v, actions...);
}

template <typename... T>
ActionList MakeActionList(T&&... actions) {
    ActionList v;
    __add_action_list(v, std::move(actions)...);
    return v;
}

}  // namespace auton
}  // namespace c2020
}  // namespace team114
