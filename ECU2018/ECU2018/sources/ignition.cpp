// 
// 
// 
// Antænd mig
#include "ignition.h"


#define ignition_pin 21 //Under antagelsen at "tænding" er ignition
#define IGNITION_CHANNEL 0
/*
Name:		Sketch2.ino
Created:	4/16/2018 9:50:54 AM
Author:	Ejer
*/

double DWELL_TIME = 3000;


void initializeIgnition() {
	pinMode(ignition_pin, OUTPUT);
	TeensyDelay::addDelayChannel(stopIgnition, IGNITION_CHANNEL);
	stopIgnition();
}

// Returns the angle from TDC in CCW degrees
double ignition_time_angle(double rpm) {
	INTERPOL IGNITION = interpolation_map(rpm);
	double xlow = ignitionArray[IGNITION.lower];
	double xhigh = ignitionArray[IGNITION.upper];
	double xinc = IGNITION.increment;
	double IGNITION_TIME = (xhigh - xlow)*xinc + xlow;
	return IGNITION_TIME;
}

double ignition_dwell_angle(double rpm) {
	return (360*rpm*DWELL_TIME)/60000000;
}
void startIgnition() {
	digitalWrite(ignition_pin, HIGH); //Sends signal to start ignition
}

void stopIgnition() {
	digitalWrite(ignition_pin, LOW); //Sends signal to stop ignition
}



bool ignitionCheck(char start_angle, char pos_angle) {
	//Check if we've reached the start angle, where we start charging the coil
	if (!digitalRead(ignition_pin) && (pos_angle >= start_angle && pos_angle <= start_angle + 5)) {
		startIgnition();
		TeensyDelay::trigger(DWELL_TIME, IGNITION_CHANNEL); 
		return false;
	}
	else {
		return true;
	}
}


