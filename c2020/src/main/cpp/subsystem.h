#pragma once

#include "util/constructor_macros.h"

namespace team114 {
namespace c2020 {

class Subsystem {
   public:
   /**
    * Subsystem is an abstract class (and all following functions are abstract as well). An empty constructor. 
    **/
    virtual ~Subsystem() = default;

    /**
    * Called in Robot::TeleopPeriodic() if it makes sense (hood and intake yes, control panel(unfinished?) and climber no)
    **/
    virtual void Periodic(){};
    /**
    * Resets things (like setting motors back to initial position)
    **/
    virtual void Stop(){};
    /**
    * Presumably to reset sensors, although it currently isn't used for much in the subclasses.
    **/
    virtual void ZeroSensors(){};
    /**
    * Currently undefined in subclasses, but it could presumably be used to output general positional or sensory data.
    **/
    virtual void OutputTelemetry(){};
};

// static member is inline so as to be single across translation units
// https://stackoverflow.com/a/53705993
#define CREATE_SINGLETON(Classname)                  \
   private:                                          \
    inline static Classname* __singleton_instance_;  \
                                                     \
   public:                                           \
    static Classname& GetInstance() {                \
        if (__singleton_instance_ == nullptr) {      \
            __singleton_instance_ = new Classname(); \
        }                                            \
        return *__singleton_instance_;               \
    }                                                \
    static void DestroyInstance() {                  \
        delete __singleton_instance_;                \
        __singleton_instance_ = nullptr;             \
    }

#define SUBSYSTEM_PRELUDE(Classname)          \
   private:                                   \
    Classname();                              \
    CREATE_SINGLETON(Classname)               \
   public: /* better diagnostics if public */ \
    DISALLOW_COPY_ASSIGN(Classname)

}  // namespace c2020
}  // namespace team114
