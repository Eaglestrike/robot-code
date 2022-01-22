
//https://docs.photonvision.org/en/latest/docs/programming/photonlib
//needs photonlib and ctre libraries


#include "Robot.h"
#include <fmt/core.h>
#include <frc/smartdashboard/SmartDashboard.h>
#include <iostream>



void Robot::TeleopInit() {
  camera.SetDriverMode(false); // we want target tracking
  camera.SetLEDMode(photonlib::LEDMode::kOn);
  camera.SetPipelineIndex(1); //test is, as far as i know, better than Reflective-Test
}


void Robot::TeleopPeriodic() {
  photonlib::PhotonPipelineResult result = camera.GetLatestResult();
  
  if (result.HasTargets()) {
    photonlib::PhotonTrackedTarget target = result.GetBestTarget();
    double yaw = target.GetYaw(); //horizontal offset (in degrees, neg is left)
    double pitch = target.GetPitch(); //vertical offset (in degrees, neg is bottom)
    double area = target.GetArea(); //0 to 100 percent of screen
    double skew = target.GetSkew(); //rotation? counter clockwise positive, from horizontal

    //distance to goal (these heights and angle are for 2022 comp)
    units::meter_t range = photonlib::PhotonUtils::CalculateDistanceToTarget(
      units::length::meter_t(18_in), units::length::meter_t(104_in), units::angle::radian_t(0.872665_rad), units::degree_t{pitch}); 

    //there's also a function that spits out a pose2d? that is like where you are on the field. might investiage that

    std::cout << "yaw: " << yaw << ", pitch: " << pitch << ", area: " << area << ", skew: " << skew << "\n";
   // std::cout << "distance to goal: "<< range.value() << "\n";
    std::cout << "\n";

  } else std::cout << "No targets\n";

}

#ifndef RUNNING_FRC_TESTS
int main() {
  return frc::StartRobot<Robot>();
}
#endif
