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

double ignition_time_angle(double rpm) {
	INTERPOL IGNITION = interpolation_map(rpm);
	double xlow = ignitionArray[IGNITION.lower];
	double xhigh = ignitionArray[IGNITION.upper];
	double xinc = IGNITION.increment;
	double IGNITION_TIME = (xhigh - xlow)*xinc + xlow;
	return IGNITION_TIME;
}

double ignition_dwell_angle(double rpm) {
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

void ignitionCheck(double start_angle, double dwell_angle, double pos_angle) {
	//Check if we've reached the start angle, where we start charging the coil
	if (!digitalRead(ignition_pin) && pos_angle >= start_angle) {
		startIgnition();
	}
	//Check if we've passed the dwell angle, where we discharge the coil
	if (digitalRead(ignition_pin) && pos_angle >= dwell_angle) {
		stopIgnition();
	}
}


