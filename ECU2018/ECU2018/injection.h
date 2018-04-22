// injection.h

#ifndef _INJECTION_h
#define _INJECTION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "ingAndinjArray.h"


extern INTERPOL injection;


//functioner
int canInjectionRun(double RPM);
uint32_t findTime(double RPM, float potentiometer);
void start();
void stop();
float calcMass(long time);
float injectionRun(double RPM, float potentiometer);
void setSlope(float newSlope);
void setInterjection(float newInterjection);
void setMAXRPM(float newMAXRPM);


#endif

