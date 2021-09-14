#pragma once

#include <frc/XboxController>

class Operator {
	public:
		Operator(Shooter s, Intake i, Climb c);
		void periodic(XboxController xbox);
		void shoot(double x);
		void intake(bool active);
		void climb(bool active);
	private:
		Shoot shoot_;
		Intake intake_;
		Climb climb_;
};