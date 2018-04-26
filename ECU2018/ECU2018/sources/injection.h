// injection.h

#ifndef _INJECTION_h
#define _INJECTION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "ingAndinjArray.h"
#include "TeensyDelay.h"

//functioner
void initializeInjection();
int canInjectionRun(double RPM);
void injectionCheck(char start, double time, char pos);
double findAngle_injection(double RPM, float potentiometer);
void startInj();
void stopInj();
float calcMass(long time);
void setSlope(float newSlope);
void setInterjection(float newInterjection);
void setMAXRPM(float newMAXRPM);


#endif

