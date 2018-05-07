// injection.h

#ifndef _INJECTION_h
#define _INJECTION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "motorLUT.h"
#include "TeensyDelay.h"

//functioner
void initializeInjection();
bool injectionCheck(int injectionStartAngle, double  injectionTime, int currentAngle);
double calculateInjectionDurationTime(double RPM, float potentiometer);
inline void startInjection();
inline void stopInjection();
float calculateConsumedFuelMass(double injectionTime);

#endif

