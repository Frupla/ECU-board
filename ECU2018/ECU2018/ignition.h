// ignition.h

#ifndef _IGNITION_h
#define _IGNITION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "ingAndinjArray.h"

#define MAX_RPM 7000
#define DWELL_TIME 5.42
double START_TIME;


//double IGNITION_TIMING;
//double IGNITION_DWELL;
//double START_TIME; //asger - Was defined multiple times

double calculate_ignition_time(double rpm);
double calculate_ignition_dwell(double rpm);
#endif

