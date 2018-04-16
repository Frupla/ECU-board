// 
// 
// 

#include "position.h"

u_int encoder_A, encoder_B, encoder_Z;
int encoder_position;

int calibration_variable;

int encoder_pin_A_intern;
int encoder_pin_B_intern;
int encoder_pin_Z_intern;

int encoderErrorCheck()
{
	return 0;
}

void initializeEncoder(int encoder_pin_A, int encoder_pin_B, int encoder_pin_Z, int calib_var)
{
	pinMode(encoder_pin_A, INPUT);
	pinMode(encoder_pin_B, INPUT);
	pinMode(encoder_pin_Z, INPUT);
	calibration_variable = calib_var;
	encoder_pin_A_intern = encoder_pin_A;
	encoder_pin_B_intern = encoder_pin_B;
	encoder_pin_Z_intern = encoder_pin_Z;
	attachInterrupt(digitalPinToInterrupt(encoder_pin_A), encoderInterrupthandlerA, RISING);
	attachInterrupt(digitalPinToInterrupt(encoder_pin_B), encoderInterrupthandlerB, RISING);
	attachInterrupt(digitalPinToInterrupt(encoder_pin_Z), encoderInterrupthandlerZ, RISING);
}

int encoderPosition_A()
{
	return encoder_A;
}

int encoderPosition_B()
{
	return encoder_B;
}

int encoderPosition_Z()
{
	return encoder_Z;
}

int encoderPositionEngine()
{
	return 0;
}

int encoderRPM()
{
	return 0;
}

void encoderInterrupthandlerA() {
	encoder_A++;
	if (digitalRead(encoder_pin_B_intern)) {

	}
}

void encoderInterrupthandlerB() {
	encoder_B++;
}

void encoderInterrupthandlerZ() {
	encoder_Z++;

}