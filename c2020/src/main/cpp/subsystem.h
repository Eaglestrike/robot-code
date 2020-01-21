#pragma once

#include "util/constructor_macros.h"

namespace team114 {
namespace c2020 {

class Subsystem {
   public:
    virtual ~Subsystem() = default;

    virtual void Periodic(){};
    virtual void Stop(){};
    virtual void ZeroSensors(){};
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
    static void DestroyInstance() { delete __singleton_instance_; }

#define SUBSYSTEM_PRELUDE(Classname)          \
   private:                                   \
    Classname();                              \
    CREATE_SINGLETON(Classname)               \
   public: /* better diagnostics if public */ \
    DISALLOW_COPY_ASSIGN(Classname)

}  // namespace c2020
}  // namespace team114
