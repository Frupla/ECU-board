// injection.h

#ifndef _INJECTION_h
#define _INJECTION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "ingAndinjArray.h"



//functioner
int canInjectionRun(double RPM);
float injectionCheck(char start, char stop, char pos);
char findAngle_injection(double RPM, float potentiometer);
void startInj();
void stopInj();
float calcMass(long time);
void setSlope(float newSlope);
void setInterjection(float newInterjection);
void setMAXRPM(float newMAXRPM);


#endif

