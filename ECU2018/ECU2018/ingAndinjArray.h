// ingAndinjArray.h

#ifndef _INGANDINJARRAY_h
#define _INGANDINJARRAY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
extern uint32_t ignitionArray[25];
extern uint32_t injectionArray[25];

#endif

