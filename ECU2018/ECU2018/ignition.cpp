// 
// 
// 
// Antænd mig
#include "ignition.h"

/*
Name:		Sketch2.ino
Created:	4/16/2018 9:50:54 AM
Author:	Ejer
*/

double calculate_ignition_time(double rpm) {
	INTERPOL IGNITION = interpolation_map(rpm);
	double xlow = ignition_Array[IGNITION.lower];
	double xhigh = ignition_Array[IGNITION.upper];
	double xinc = IGNITION.increment;
	double IGNITION_TIME = (xhigh - xlow)*xinc + xlow;
	return IGNITION_TIME;
}

double calculate_ignition_dwell(double rpm) {
	double dwell_angle = (360 * rpm*DWELL_TIME) / 60000;
	return dwell_angle;
}

