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

float DWELL_TIME = 3000;


void initializeIgnition() {
	pinMode(ignition_pin, OUTPUT);
	TeensyDelay::addDelayChannel(stopIgnition, IGNITION_CHANNEL);
	stopIgnition();
}

char ignition_time_angle(double rpm) {
	INTERPOL IGNITION = interpolation_map(rpm);
	double xlow = ignitionArray[IGNITION.lower];
	double xhigh = ignitionArray[IGNITION.upper];
	double xinc = IGNITION.increment;
	char IGNITION_TIME = (xhigh - xlow)*xinc + xlow;
	return IGNITION_TIME;
}

char ignition_dwell_angle(double rpm) {
	char dwell_angle = (360 * rpm*DWELL_TIME) / 60000;
	return dwell_angle;
}
void startIgnition() {
	digitalWrite(ignition_pin, HIGH); //Sends signal to start ignition
}

void stopIgnition() {
	digitalWrite(ignition_pin, LOW); //Sends signal to stop ignition
}



void ignitionCheck(char start_angle, char pos_angle) {
	//Check if we've reached the start angle, where we start charging the coil
	if (!digitalRead(ignition_pin) && (pos_angle >= start_angle && pos_angle <= start_angle + 5)) {
		startIgnition();
		TeensyDelay::trigger(DWELL_TIME, IGNITION_CHANNEL); 
	}
	//Check if we've passed the dwell angle, where we discharge the coil
	//if (digitalRead(ignition_pin) && pos_angle >= dwell_angle) {
	//	stopIgnition();
	//}
}


