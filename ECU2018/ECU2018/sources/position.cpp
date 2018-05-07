// 
// Asger (s163905) 
// 

#include "position.h"

//#define A_PULSE 29 // A pulse (set in QuadDecode)
//#define B_PULSE 30 // B pulse (set in QuadDecode)
#define Z_PULSE_PIN 28 // Z pulse

uint32_t zPulseCount;

int encoderTdcOffset;
bool zPulseFlag;

uint32_t zPulseTime;
uint32_t previousZPulseTime;

QuadDecode_t QuadDecode;
// Encoder 1 er wheelsensor
// Encoder 2 er motor RPM

void initializeEncoder(int _encoderTdcOffset) {
	// Set pinmode
	pinMode(Z_PULSE_PIN, INPUT);
	// Write to intern variables
	encoderTdcOffset = _encoderTdcOffset;
	// Reset intern variables
	zPulseCount = 0;
	zPulseTime = 0;
	previousZPulseTime = 0;
	zPulseFlag = false;
	// Attach interrupt
	attachInterrupt(digitalPinToInterrupt(Z_PULSE_PIN), zPulseInterruptHandler, RISING);
}


int encoderPositionEngine() {
	if (QuadDecode.getCounter2() - encoderTdcOffset  < 0) {
		return (QuadDecode.getCounter2() / 4) + 360 - encoderTdcOffset; 
	}
	else {
		return (QuadDecode.getCounter2() / 4) - 360 - encoderTdcOffset;
	}
	// Divide by 4, because the hardware encoder counts on change on both channels. (4 counts per pulse)
	// % by 720 for the calibration variable.
}

int encoderPositionWheel() {
	return QuadDecode.getCounter1() / 4;
	// Divide by 4, because the hardware encoder counts on change on both channels. (4 counts per pulse)
}

void resetEncoderWheel(int resetValue) {
	QuadDecode.setCounter1(resetValue);
}

void zPulseInterruptHandler() {
	zPulseCount++;
	QuadDecode.setCounter2(0); // correcting so TDC2 is 0
	previousZPulseTime = zPulseTime;
	zPulseTime = micros();
	zPulseFlag = true;
}

float encoderRPM() { // Returns the RPM. Returns -1 if fail, so every ~70min and startup
	if (zPulseTime - previousZPulseTime <= 0) {
		return -1;
	}
	float RPM = (120000000 / (float)(zPulseTime - previousZPulseTime));
	// T = encoder_Z_time - encoder_Z_time_old // How many µs between 2 ticks
	// 60 s/min * 10e6 µs/s  /  T µs   =  60000000 / T  rounds/min
	return RPM;
}

bool getZPulseFlag() {
	return zPulseFlag;
}

void setZPulseFlag(bool value) {
	zPulseFlag = value;
}

// TODO: errorchecking with the Encoder library, to be done regularily, or as a seperate "errorcheck" file