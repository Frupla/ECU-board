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

void initializeEncoder(int encoder_pin_A, int encoder_pin_B, int encoder_pin_Z, int calibration_variable);
int encoderPosition_A();
int encoderPosition_B();
int encoderPosition_Z();

#endif

