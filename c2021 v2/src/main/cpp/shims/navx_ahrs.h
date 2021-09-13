#pragma once

// Kuailabs doesnt supply x86 binaries in their package.
// This header enables building on x86 by replacing the arm-only class
// with an empty interface

#ifdef __arm__

#include "AHRS.h"

#else

#include "frc/I2C.h"
#include "frc/PIDSource.h"
#include "frc/SPI.h"
#include "frc/SerialPort.h"
#include "frc/smartdashboard/SendableBase.h"
#include "frc/smartdashboard/SendableBuilder.h"

class AHRS /*: public frc::SendableBase,
             public frc::ErrorBase,
             public frc::PIDSource*/
{
   public:
    enum BoardAxis {
        kBoardAxisX = 0,
        kBoardAxisY = 1,
        kBoardAxisZ = 2,
    };

    struct BoardYawAxis {
        /* Identifies one of the board axes */
        BoardAxis board_axis;
        /* true if axis is pointing up (with respect to gravity); false if
         * pointing down. */
        bool up;
    };

    enum SerialDataType {
        /**
         * (default):  6 and 9-axis processed data
         */
        kProcessedData = 0,
        /**
         * unprocessed data from each individual sensor
         */
        kRawData = 1
    };

   public:
    AHRS(frc::SPI::Port spi_port_id) {}
    AHRS(frc::I2C::Port i2c_port_id) {}
    AHRS(frc::SerialPort::Port serial_port_id) {}

    AHRS(frc::SPI::Port spi_port_id, uint8_t update_rate_hz) {}
    AHRS(frc::SPI::Port spi_port_id, uint32_t spi_bitrate,
         uint8_t update_rate_hz) {}

    AHRS(frc::I2C::Port i2c_port_id, uint8_t update_rate_hz) {}

    AHRS(frc::SerialPort::Port serial_port_id, AHRS::SerialDataType data_type,
         uint8_t update_rate_hz) {}

    float GetPitch() { return 0.0; }
    float GetRoll() { return 0.0; }
    float GetYaw() { return 0.0; }
    float GetCompassHeading() { return 0.0; }
    void ZeroYaw() {}
    bool IsCalibrating() { return false; }
    bool IsConnected() { return false; }
    double GetByteCount() { return 0.0; }
    double GetUpdateCount() { return 0.0; }
    long GetLastSensorTimestamp() { return 0; }
    float GetWorldLinearAccelX() { return 0.0; }
    float GetWorldLinearAccelY() { return 0.0; }
    float GetWorldLinearAccelZ() { return 0.0; }
    bool IsMoving() { return false; }
    bool IsRotating() { return false; }
    float GetBarometricPressure() { return 0.0; }
    float GetAltitude() { return 0.0; }
    bool IsAltitudeValid() { return false; }
    float GetFusedHeading() { return 0.0; }
    bool IsMagneticDisturbance() { return false; }
    bool IsMagnetometerCalibrated() { return false; }
    float GetQuaternionW() { return 0.0; }
    float GetQuaternionX() { return 0.0; }
    float GetQuaternionY() { return 0.0; }
    float GetQuaternionZ() { return 0.0; }
    void ResetDisplacement() {}
    void UpdateDisplacement(float accel_x_g, float accel_y_g,
                            int update_rate_hz, bool is_moving);
    float GetVelocityX() { return 0.0; }
    float GetVelocityY() { return 0.0; }
    float GetVelocityZ() { return 0.0; }
    float GetDisplacementX() { return 0.0; }
    float GetDisplacementY() { return 0.0; }
    float GetDisplacementZ() { return 0.0; }
    double GetAngle() { return 0.0; }
    double GetRate() { return 0.0; }
    void SetAngleAdjustment(double angle) {}
    double GetAngleAdjustment() { return 0.0; };
    void Reset() {}
    float GetRawGyroX() { return 0.0; };
    float GetRawGyroY() { return 0.0; };
    float GetRawGyroZ() { return 0.0; };
    float GetRawAccelX() { return 0.0; };
    float GetRawAccelY() { return 0.0; };
    float GetRawAccelZ() { return 0.0; };
    float GetRawMagX() { return 0.0; };
    float GetRawMagY() { return 0.0; };
    float GetRawMagZ() { return 0.0; };
    float GetPressure() { return 0.0; };
    float GetTempC() { return 0.0; };
    AHRS::BoardYawAxis GetBoardYawAxis() {
        return {BoardAxis::kBoardAxisX, true};
    }
    std::string GetFirmwareVersion() { return ""; };

    // bool RegisterCallback(ITimestampedDataSubscriber *callback,
    //                       void *callback_context) {
    //     return false;
    // }
    // bool DeregisterCallback(ITimestampedDataSubscriber *callback) {
    //     return false;
    // }

    int GetActualUpdateRate() { return 0; }
    int GetRequestedUpdateRate() { return 0; }

    void EnableLogging(bool enable) {}
    void EnableBoardlevelYawReset(bool enable) {}
    bool IsBoardlevelYawResetEnabled() { return false; }

    int16_t GetGyroFullScaleRangeDPS() { return 0; }
    int16_t GetAccelFullScaleRangeG() { return 0; }
};

#endif