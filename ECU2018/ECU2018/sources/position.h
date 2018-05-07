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

void initializeEncoder(int _encoderTdcOffset);
int encoderPositionEngine();
float encoderRPM();
void zPulseInterruptHandler();

int encoderPositionWheel();
void resetEncoderWheel(int resetValue);

bool getZPulseFlag();
void setZPulseFlag(bool value);

#endif

