// position.h

#ifndef _POSITION_h
#define _POSITION_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// We should probalby move this to the main?

// Magic numbers


void initializeEncoder(uint8_t encoder_pin_A, uint8_t encoder_pin_B, uint8_t encoder_pin_Z, int calib_var);
int encoderPosition_A();
int encoderPosition_B();
int encoderPosition_Z();
int encoderPositionEngine();
float encoderRPM();
int encoderErrorCheck();
void encoderInterrupthandlerA();
void encoderInterrupthandlerB();
void encoderInterrupthandlerZ();

#endif

