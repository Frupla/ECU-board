// ignition.h

#ifndef _IGNITION_h
#define _IGNITION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "motorLUT.h"
#include "TeensyDelay.h"

void initializeIgnition();
double calculateIgnitionStopAngle(double rpm);
double calculateDwellAngle(double rpm);
inline void startIgnition();
inline void stopIgnition();
bool ignitionCheck(int ignitionStartAngle, int currentAngle);
#endif

