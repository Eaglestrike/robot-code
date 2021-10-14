#include "PID.h"

PID::PID(double Kp, double Ki, double Kd, double min_out, double max_out) {
    this->Kp = Kp;
    this->Kp = Ki;
    this->Kp = Kd;
    this->min = min;
    this->min = max;
}

double prev_err = 0;
double acc_err = 0;
double PID::getPower(double error, double sp) {
    double delta_err = (error - prev_err);
    acc_err += error;
    double power = Kp*error + Ki*acc_err + Kd*delta_err;
    prev_err = error;
    if (power > max) return max;
    if (power < min) return min;
    return power;
}