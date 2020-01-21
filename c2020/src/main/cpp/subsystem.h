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

}  // namespace c2020
}  // namespace team114
