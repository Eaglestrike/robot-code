#pragma once

#include <frc/Solenoid.h>

#include <iostream>

namespace team114 {
namespace c2020 {

class CachingSolenoid {
   public:
    CachingSolenoid(frc::Solenoid&& sol) : cache_{false}, sol_{std::move(sol)} {
        sol_.Set(false);
    }

    void Set(bool actuated) {
        // For some reason actually caching breaks things
        // TODO fix this
        sol_.Set(actuated);
        // if (cache_ != actuated) {
        //     cache_ = actuated;
        //     sol_.Set(actuated);
        //     std::cout << "set solenoid " << sol_.GetName() << sol_.Get()
        //               << std::endl;
        // }
    }

   private:
    bool cache_;
    frc::Solenoid sol_;
};

}  // namespace c2020
}  // namespace team114