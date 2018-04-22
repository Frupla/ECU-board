// 
// 
// 
// Antænd mig
#include "ignition.h"

#define ignition_pin 21 //Under antagelsen at "tænding" er ignition

/*
Name:		Sketch2.ino
Created:	4/16/2018 9:50:54 AM
Author:	Ejer
*/

float MAXRPM;
float DWELL_TIME = 5.42;

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
void startIgnition() {
	digitalWrite(ignition_pin, HIGH); //Sends signal to start ignition
}

void stopIgnition() {
	digitalWrite(ignition_pin, LOW); //Sends signal to stop ignition
}

int canIgnitionRun(double RPM) {
	if (RPM > MAXRPM) {
		return 0;
	}
	else {
		return 1;
	}
}

void ignitionRun(double RPM) {
	if (canIgnitionRun(RPM)) {
		stopIgnition();		//Stop the ignition

		//Calculate charge start time AFTER the current micros().
		START_TIME = calculate_ignition_time(RPM) - calculate_ignition_dwell(RPM) + micros();
		//Wait for charge start
		while(1!=2){
			if (micros() >= START_TIME) {
				break;
			}
		}
		//Turn on ignition
		startIgnition();

		//Wait for ignition time
		while (1!=2) {
			if (micros() >= START_TIME+DWELL_TIME) {
				break;
			}
		}
		stopIgnition();
	}
}

