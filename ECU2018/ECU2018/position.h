// position.h

#ifndef _POSITION_h
#define _POSITION_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// We should probalby move this to the main?
#define ENCODER_Z_PIN 28
#define ENCODER_A_PIN 29
#define ENCODER_B_PIN 30
// Magic numbers


void initializeEncoder(uint8_t encoder_pin_A, uint8_t encoder_pin_B, uint8_t encoder_pin_Z, int calib_var);
int encoderPosition_A();
int encoderPosition_B();
int encoderPosition_Z();
int encoderPositionEngine();
int encoderRPM();

#endif

