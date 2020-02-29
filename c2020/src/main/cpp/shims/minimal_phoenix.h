#pragma once

// this is a comproimise
// CTRE uses back-asswards namespace conventions,
// like we're in Java land, basically mandating we use
// using namespace declarations.
// The Phoenix.h header does this, but also brings in the whole library
// In reality, we need like two classes and associated data types.

// Since basically every translation unit will have ctre stuff,
// this should reduce compilation times

#include <ctre/phoenix/motorcontrol/can/TalonFX.h>
#include <ctre/phoenix/motorcontrol/can/TalonSRX.h>

using namespace ctre;
using namespace ctre::phoenix;
using namespace ctre::phoenix::motion;
using namespace ctre::phoenix::motorcontrol;
using namespace ctre::phoenix::motorcontrol::can;
using namespace ctre::phoenix::sensors;
