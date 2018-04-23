// position.h

#ifndef _POSITION_h
#define _POSITION_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "Encoder-master/Encoder.h"
#include "autoGear.h"
// We should probalby move this to the main?

// Magic numbers

//our own:
void initializeEncoder(uint8_t encoder_pin_A, uint8_t encoder_pin_B, uint8_t encoder_pin_Z, int calib_var);
int encoderPosition_A();
int encoderPosition_B();
int encoderPosition_Z();
int32_t encoderPositionEngine();
float encoderRPM();
int encoderErrorCheck();
void encoderInterrupthandlerA();
void encoderInterrupthandlerB();
void encoderInterrupthandlerZ();

bool getzPulseFlag();
void setzPulseFlag(bool what_to_be);

//using Encoder lib
void altInitializeEncoder(uint8_t encoder_pin_A, uint8_t encoder_pin_B, uint8_t encoder_pin_Z, int calib_var);
int altEncoderPositionEngine();
void altEncoderInterrupthandlerZ();

#endif

