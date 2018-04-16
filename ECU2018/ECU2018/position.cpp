// 
// 
// 

#include "position.h"

int Encoder::errorcheck()
{
	return 0;
}

void Encoder::A_count()
{
	encoder_A++;
}

Encoder::Encoder(int encoder_pin_A, int encoder_pin_B, int encoder_pin_Z, int calibration_variable)
{
	pinMode(encoder_pin_A, INPUT);
	pinMode(encoder_pin_B, INPUT);
	pinMode(encoder_pin_Z, INPUT);
	attachInterrupt(digitalPinToInterrupt(encoder_pin_A), A_count, RISING);

}

int Encoder::position_A()
{
	return 0;
}

int Encoder::position_B()
{
	return 0;
}

int Encoder::position_Z()
{
	return 0;
}

int Encoder::position_engine()
{
	return 0;
}

int Encoder::RPM()
{
	return 0;
}

void A_interrupthandler() {

}


