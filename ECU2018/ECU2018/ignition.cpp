// 
// 
// 
// Antænd mig
#include "ignition.h"

double calculate_ignition_time(double rpm) {
	INTERPOL IGNITION = interpolation_map(rpm);
	double xlow = ignitionArray[IGNITION.lower];
	double xhigh = ignitionArray[IGNITION.upper];
	double xinc = IGNITION.increment;
	double IGNITION_TIME = (xhigh - xlow)*xinc + xlow;
	return IGNITION_TIME;
}

double calculate_ignition_dwell(double rpm) {
	double dwell_angle = (360 * rpm*DWELL_TIME) / 60000;
	return dwell_angle;
}

