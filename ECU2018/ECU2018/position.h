// position.h

#ifndef _POSITION_h
#define _POSITION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define ENCODER_Z_PIN 28
#define ENCODER_A_PIN 29
#define ENCODER_B_PIN 30

Encoder * encoder;


class Encoder {
private:
	volatile int encoder_A, encoder_B, encoder_Z;
	int errorcheck();
	void A_count();
public:
	Encoder(int encoder_pin_A, int encoder_pin_B, int encoder_pin_Z, int calibration_variable);
	int position_A();
	int position_B();
	int position_Z();
	int position_engine();
	int RPM();
};


#endif

