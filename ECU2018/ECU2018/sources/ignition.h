// ignition.h

#ifndef _IGNITION_h
#define _IGNITION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "ingAndinjArray.h"
#include "TeensyDelay.h"

//double START_TIME;


//double IGNITION_TIMING;
//double IGNITION_DWELL;
//double START_TIME; //asger - Was defined multiple times

void initializeIgnition();
double ignition_time_angle(double rpm);
double ignition_dwell_angle(double rpm);
void startIgnition();
void stopIgnition();
bool ignitionCheck(char start_angle, char pos_angle);
#endif

