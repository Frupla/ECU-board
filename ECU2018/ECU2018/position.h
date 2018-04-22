// position.h

#ifndef _POSITION_h
#define _POSITION_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Encoder.h"
// We should probalby move this to the main?

// Magic numbers

//our own:
void initializeEncoder(uint8_t encoder_pin_A, uint8_t encoder_pin_B, uint8_t encoder_pin_Z, int calib_var);
int encoderPosition_A();
int encoderPosition_B();
int encoderPosition_Z();
int encoderPositionEngine();
bool friskFlag69; // tak frederik
float encoderRPM();
int encoderErrorCheck();
void encoderInterrupthandlerA();
void encoderInterrupthandlerB();
void encoderInterrupthandlerZ();

//using Encoder lib
void altInitializeEncoder(uint8_t encoder_pin_A, uint8_t encoder_pin_B, uint8_t encoder_pin_Z, int calib_var);
int altEncoderPositionEngine();
void altEncoderInterrupthandlerZ();

#endif

