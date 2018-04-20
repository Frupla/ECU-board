// ignition.h

#ifndef _IGNITION_h
#define _IGNITION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
typedef struct INTERPOLATION_CONSTANTS {
	uint lower;
	uint upper;
	uint close;
	double increment;
} INTERPOL;

#define MAX_RPM 7000
#define DWELL_TIME 5.42
double RPM = 0;
int RUN_IGNITION;
int IGNITION_OFF = 0;
int ignition_Array[25];

double IGNITION_TIMING;
double IGNITION_DWELL;
double START_TIME;

double calculate_ignition_time(double rpm);
double calculate_ignition_dwell(double rpm);
#endif

