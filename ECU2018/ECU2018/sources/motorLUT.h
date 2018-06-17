// ingAndinjArray.h, yes this is the optimal name

#ifndef _INGANDINJARRAY_h
#define _INGANDINJARRAY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


typedef struct INTERPOL {
	int lower;
	int upper;
	double increment;
} INTERPOL_t;

extern uint32_t ignitionArray[25];
extern uint32_t injectionArray[25];
INTERPOL_t calculateInterpolation(double rpm);

#endif

