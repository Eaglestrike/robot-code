#pragma once

#include <frc/Solenoid.h>

namespace team114 {
namespace c2020 {

class CachingSolenoid {
   public:
    CachingSolenoid(frc::Solenoid&& sol) : cache_{false}, sol_{std::move(sol)} {
        sol_.Set(false);
    }

    void Set(bool actuated) {
        if (cache_ == actuated) {
            return;
        }
        cache_ = actuated;
        sol_.Set(cache_);
    }

   private:
    bool cache_;
    frc::Solenoid sol_;
};

}  // namespace c2020
}  // namespace team114
