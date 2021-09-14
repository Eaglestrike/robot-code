#include <Operator.h>

Operator::Operator(Shoot s, Intake i, Climb c) {
	this.shoot_ = s;
	this.intake_ = i;
	this.climb_ = c;
}

void Operator::periodic(frc::XboxController xbox){
	shooter_.periodic();
	intake_.periodic();
	climb_.periodic();
	//condition for climb
	climb(xbox.getAButtonPressed());
	//condition for intake
	intake(xbox.getBButtonPressed());
	//condition for shoot
	shoot(xbox.getX());
}
void Operator::shoot(double x){
	//Numbers are made up. I don't know what they mean.
	if(x > 0.8){
		//calling shooter to do things
		//can be changed later.
		shoot_.setWantState(Shoot::State::LongShot);
	}
	else if(x > 0.5){
		shoot_.setWantState(Shoot::State::MediumShot);
	}
	else if(x > 0.1){
		shoot_.setWantState(Shoot::State::ShortShot);
	}
	else{
		shoot_.setWantState(Shoot::State::Idle);
	}
}
void Operator::climb(bool active){
	if(active){
		//Calling Climb to do things, can be changed later
		climb_.setWantState(Climb::State::Climbing);
	}
	else{
		climb_.setWantState(Climb::State::Idle);
	}
}
void Operator::intake(bool active){
	if(active){
		//Calling Climb to do things, can be changed later
		intake_.setWantState(Intake::State::Running);
	}
	else{
		intake_.setWantState(Intake::State::Idle);
	}
}