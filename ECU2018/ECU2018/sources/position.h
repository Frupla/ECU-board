// position.h

#ifndef _POSITION_h
#define _POSITION_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


#include "autoGear.h"
#include "QuadDecodeSimple.h"

// We should probalby move this to the main?

// Magic numbers

//our own:
int32_t return_debug_variable();

void initializeEncoder(uint8_t encoder_pin_Z, int calib_var);
int altEncoderPosition_A();
int altEncoderPosition_B();
int EncoderPosition_Z();
int16_t encoderPositionEngine();
float encoderRPM();
int altEncoderErrorCheck();
void altEncoderInterrupthandlerA();
void altEncoderInterrupthandlerB();
void altEncoderInterrupthandlerZ();
void encoderInterrupthandlerZ();

int16_t encoderPositionWheel();
void resetEncoderWheel(int16_t reset_to);

bool getzPulseFlag();
void setzPulseFlag(bool what_to_be);

//using Encoder lib
void altInitializeEncoder(uint8_t encoder_pin_A, uint8_t encoder_pin_B, uint8_t encoder_pin_Z, int calib_var);
int32_t altEncoderPositionEngine();
void altEncoderInterrupthandlerZ();

#endif

