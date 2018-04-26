// 
// Asger (s163905) 
// 

#include "position.h"


u_int encoder_A, encoder_B, encoder_Z;

int calibration_variable;
int32_t encoder_position;
bool zPulseFlag;

uint8_t encoder_pin_A_intern;
uint8_t encoder_pin_B_intern;
uint8_t encoder_pin_Z_intern;

uint32_t encoder_Z_time;
uint32_t encoder_Z_time_old;

QuadDecode_t QuadDecode;
// coder 1 er wheelsensor
// coder 2 er motor RPM

int16_t encoderPositionEngine() {
	return (QuadDecode.getCounter2()/4) % 720; // TODO: Same as the other todo
	// Divide by 4, because the hardware encoder counts on change on both channels. (4 counts per pulse)
	// % by 720 for the calibration variable.
}

int16_t encoderPositionWheel() {
	return QuadDecode.getCounter1()/4;
	// Divide by 4, because the hardware encoder counts on change on both channels. (4 counts per pulse)
}

void resetEncoderWheel(int16_t reset_to) {
	QuadDecode.setCounter1(reset_to);
}

// code written all on our lonesome
int altEncoderErrorCheck() // Returnerer EXIT_FAILURE hvis der er forskydning mellem A og B
{
	if ((int)encoder_A - (int)encoder_B > 1 || (int)encoder_A - (int)encoder_B < 1) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
} // er det en god ide at returnerer det EXIT_FAILURE? stopper det ikke bare koden?

void encoderInterrupthandlerZ() {
	encoder_Z++;
	QuadDecode.setCounter2(calibration_variable); // correcting so TDC2 is 0 // TODO: This feels wrong, checkup
	encoder_Z_time_old = encoder_Z_time;
	encoder_Z_time = micros();
	zPulseFlag = true;
}

void initializeEncoder(uint8_t encoder_pin_Z, int calib_var)
{
	// Set pinmode
	pinMode(encoder_pin_Z, INPUT);
	// Write to intern variables
	calibration_variable = calib_var;
	encoder_pin_Z_intern = encoder_pin_Z;
	// Reset intern variables
	encoder_Z = 0;
	encoder_Z_time = 0;
	encoder_Z_time_old = 0;
	zPulseFlag = false;
	// Attach interrupts
	attachInterrupt(digitalPinToInterrupt(encoder_pin_Z), encoderInterrupthandlerZ, RISING);
}

void altInitializeEncoder(uint8_t encoder_pin_A, uint8_t encoder_pin_B, uint8_t encoder_pin_Z, int calib_var)
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
	zPulseFlag = false;
	// Attach interrupts
	attachInterrupt(digitalPinToInterrupt(encoder_pin_A), altEncoderInterrupthandlerA, RISING);
	attachInterrupt(digitalPinToInterrupt(encoder_pin_B), altEncoderInterrupthandlerB, RISING);
	attachInterrupt(digitalPinToInterrupt(encoder_pin_Z), altEncoderInterrupthandlerZ, RISING);
}

int altEncoderPosition_A()
{
	return encoder_A;
}

int altEncoderPosition_B()
{
	return encoder_B;
}

int altEncoderPosition_Z()
{
	return encoder_Z;
}

int32_t altEncoderPositionEngine()
{	
	return encoder_position;
}

float encoderRPM() { // Returns the RPM. Returns -1 if fail, so every ~70min and startup
	if (encoder_Z_time - encoder_Z_time_old <= 0) {
		return -1;
	}
	float output_RPM = (60000000 / (float)(encoder_Z_time - encoder_Z_time_old));
	// T = encoder_Z_time - encoder_Z_time_old // How many µs between 2 ticks
	// 60 s/min * 10e6 µs/s  /  T µs   =  60000000 / T  rounds/min
	return output_RPM;
}

void altEncoderInterrupthandlerA() {
	encoder_A++;
	if (digitalRead(encoder_pin_B_intern)) {
		encoder_position++;
	}
	else {
		encoder_position--;
	}

}

void altEncoderInterrupthandlerB() {
	encoder_B++;
}

void altEncoderInterrupthandlerZ() {
	encoder_Z++;
	encoder_A = 0;
	encoder_B = 0;
	encoder_position = calibration_variable; // correcting so TDC2 is 0
	encoder_Z_time_old = encoder_Z_time;
	encoder_Z_time = micros();
	zPulseFlag = true;
}


bool getzPulseFlag() {
	return zPulseFlag;
}

void setzPulseFlag(bool what_to_be) {
	zPulseFlag = what_to_be;
}



// TODO: errorchecking with the Encoder library, to be done regularily, or as a seperate "errorcheck" file