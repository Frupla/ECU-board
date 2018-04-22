// 
// Asger (s163905) 
// 

#include "position.h"


u_int encoder_A, encoder_B, encoder_Z;

int calibration_variable;
int encoder_position;

uint8_t encoder_pin_A_intern;
uint8_t encoder_pin_B_intern;
uint8_t encoder_pin_Z_intern;

uint32_t encoder_Z_time;
uint32_t encoder_Z_time_old;

Encoder * autoPosition;

// code written all on our lonesome
int encoderErrorCheck() // Returnerer EXIT_FAILURE hvis der er forskydning mellem A og B
{
	if ((int)encoder_A - (int)encoder_B > 1 || (int)encoder_A - (int)encoder_B < 1) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void initializeEncoder(uint8_t encoder_pin_A, uint8_t encoder_pin_B, uint8_t encoder_pin_Z, int calib_var)
{
	// Set pinmode
	pinMode(encoder_pin_A, INPUT);
	pinMode(encoder_pin_B, INPUT);
	pinMode(encoder_pin_Z, INPUT);
	// Write to intern variables
	calibration_variable = calib_var;
	encoder_pin_A_intern = encoder_pin_A;
	encoder_pin_B_intern = encoder_pin_B;
	encoder_pin_Z_intern = encoder_pin_Z;
	// Reset intern variables
	encoder_A = 0; encoder_B = 0; encoder_Z = 0;
	encoder_position = calib_var;
	encoder_Z_time = 0;
	encoder_Z_time_old = 0;
	// Attach interrupts
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
	return encoder_position;
}

float encoderRPM() { // Returns the RPM. Returns -1 every ~70min
	if (encoder_Z_time - encoder_Z_time_old <= 0) {
		return -1;
	}
	float output_RPM = (60000000 / (float)(encoder_Z_time - encoder_Z_time_old));
	// T = encoder_Z_time - encoder_Z_time_old // How many µs between 2 ticks
	// 60 s/min * 10e6 µs/s  /  T µs   =  60000000 / T  rounds/min
	return output_RPM;
}


void encoderInterrupthandlerA() {
	encoder_A++;
	if (digitalRead(encoder_pin_B_intern)) {
		encoder_position++;
	}
	else {
		encoder_position--;
	}

}

void encoderInterrupthandlerB() {
	encoder_B++;
}

void encoderInterrupthandlerZ() {
	encoder_Z++;
	encoder_A = 0;
	encoder_B = 0;
	encoder_position = calibration_variable; // correcting so TDC2 is 0
	encoder_Z_time_old = encoder_Z_time;
	encoder_Z_time = micros();
}

// code written to use the Encoder library

void altInitializeEncoder(uint8_t encoder_pin_A, uint8_t encoder_pin_B, uint8_t encoder_pin_Z, int calib_var)
{
	// Set pinmode
	pinMode(encoder_pin_Z, INPUT);
	// Write to intern variables
	calibration_variable = calib_var;
	encoder_pin_Z_intern = encoder_pin_Z;
	// Reset intern variables
	autoPosition = new Encoder(encoder_pin_A, encoder_pin_B);
	encoder_Z = 0;
	encoder_Z_time = 0;
	encoder_Z_time_old = 0;
	attachInterrupt(digitalPinToInterrupt(encoder_pin_Z), altEncoderInterrupthandlerZ, RISING);
}

int altEncoderPositionEngine()
{
	return autoPosition->read();
}	

void altEncoderInterrupthandlerZ() {
	encoder_Z++;
	autoPosition->write(calibration_variable); // hopefully correcting so TDC2 is 0
	encoder_Z_time_old = encoder_Z_time;
	encoder_Z_time = micros();
}

// TODO: errorchecking with the Encoder library, to be done regularily
// TODO: code to include this in ECU2018 main file 