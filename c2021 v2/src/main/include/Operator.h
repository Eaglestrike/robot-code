#pragma once

#include <frc/XboxController>

class Operator {
	public:
		Operator(Shooter s, Intake i, Climb c);
		void periodic(XboxController xbox);
		void shoot();
		void intake();
		void climb();
	private:
		Shoot shoot_;
		Intake intake_;
		Climb climb_;
};