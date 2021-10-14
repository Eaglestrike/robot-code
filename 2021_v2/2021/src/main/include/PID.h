
class PID {
    public:
        PID(double Kp, double Ki, double Kd, double min_out, double max_out);
        double getPower(double error, double sp);
    private:
        double Kp, Ki, Kd, min, max;
};