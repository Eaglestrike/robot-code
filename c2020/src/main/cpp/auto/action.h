#pragma once

#include "../util/constructor_macros.h"

namespace team114 {
namespace c2020 {
namespace auton {

class Action {
   public:
    Action() = default;
    Action(Action&&) = default;
    DISALLOW_COPY_ASSIGN(Action)

    // Run once when action first scheduled
    virtual void Start() {}

    // Iterate
    // This function is not guaranteed to run if Finished() returns true early
    virtual void Periodic() {}

    // Informs the executor whether the action has completed.
    // The action will be unscheduled once this returns true.
    virtual bool Finished() { return true; };

    // Cleanup when action is un-scheduled, either due to completion or
    // termination. Other methods must not be invoked after Stop() is called.
    // Stop() is guaranteed to be invoked shortly after Finished() returns true.
    virtual void Stop() {}
};

}  // namespace auton
}  // namespace c2020
}  // namespace team114
