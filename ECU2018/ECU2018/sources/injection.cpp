// Frederik Ettrup Larsen s163920
// 20/4/2018
// 

#include "injection.h"

#define inject_pin 20//Might be 20, if it is "dyse" which I'm assuming it is
//Constants
//for the calculation of the mass of fuel based on time
float slope;
float interjection;
// m = slope * t + interjection
float MAXRPM; //Should be given by the mek's

INTERPOL injection;

void initializeInjection() {
	TeensyDelay::addDelayChannel(stopInj(), inject_pin);
}

void setSlope(float newSlope) {
	slope = newSlope;
}

void setInterjection(float newInterjection) {
	interjection = newInterjection;
}

void setMAXRPM(float newMAXRPM) {
	MAXRPM = newMAXRPM;
}

int canInjectionRun(double RPM) {
	if (RPM > MAXRPM){
		return 0;
	}
	else {
		return 1;
	}
}


double findAngle_injection(double RPM, float potentiometer) {
	injection = interpolation_map(RPM);
	uint xhigh = (uint)injectionArray[injection.upper];
	uint xlow = (uint)injectionArray[injection.lower];
	double xinc = injection.increment;
	double time = (xhigh - xlow)*xinc + xlow;
	return time*potentiometer;
}

void startInj(){
	digitalWrite(inject_pin, HIGH); //Sends signal to start injection
}

void stopInj() {
	digitalWrite(inject_pin, LOW); //Sends signal to stop injection
}

float calcMass(long angle){
	return slope * angle + interjection; //calculate mass of fuel based on angle which correlates to time trust me guys
}

void injectionCheck(char startAngle, double  time, char posAngle){
	if (!digitalRead(inject_pin) && posAngle >= startAngle) {
		startInj();
		TeensyDelay::trigger(time, inject_pin);
	}
}