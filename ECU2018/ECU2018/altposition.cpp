// 
// 
// 

#include "altposition.h"
#include <Encoder.h>


u_int encoder_A, encoder_B, encoder_Z;

int calibration_variable;
int encoder_position;

uint8_t encoder_pin_A_intern;
uint8_t encoder_pin_B_intern;
uint8_t encoder_pin_Z_intern;

uint32_t encoder_Z_time;
uint32_t encoder_Z_time_old;

void altInitializeEncoder(uint8_t encoder_pin_A, uint8_t encoder_pin_B, uint8_t encoder_pin_Z, int calib_var)
{
	// Set pinmode
	pinMode(encoder_pin_Z, INPUT);
	// Write to intern variables
	calibration_variable = calib_var;
	encoder_pin_Z_intern = encoder_pin_Z;
	// Reset intern variables
	Encoder autoPosition(encoder_pin_A, encoder_pin_B);
	encoder_A = 0; encoder_B = 0; encoder_Z = 0;
	encoder_position = 0;
	encoder_Z_time = 0;
	encoder_Z_time_old = 0;
}

int altEncoderPositionEngine()
{
	return autoPosition.read(); // hopefully correcting to TDC2 is 0
}

void encoderInterrupthandlerZ() {
	encoder_Z++;
	autoPosition.write = calibration_variable;
	encoder_Z_time_old = encoder_Z_time;
	encoder_Z_time = micros();
}