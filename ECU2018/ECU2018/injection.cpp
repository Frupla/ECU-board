// Frederik Ettrup Larsen s163920
// 20/4/2018
// 

#include "injection.h"

#define inject_pin 20//Might be 20, if it is "dyse" which I'm assuming it is

int canInjectionRun(double RPM) {
	if (RPM > MAXRPM){
		return 0;
	}
	else {
		return 1;
	}
}


uint32_t findTime(double RPM, float potentiometer) {
	injection = interpolation_map(RPM);
	long time;
	uint xhigh = (uint)injectionArray[injection.upper];
	uint xlow = (uint)injectionArray[injection.lower];
	double xinc = injection.increment;
	time = (long)(xhigh - xlow)*xinc + xlow;
	return time*potentiometer;
}

void start(){
	digitalWrite(inject_pin, HIGH); //Sends signal to start injection
}

void stop() {
	digitalWrite(inject_pin, LOW); //Sends signal to stop injection
}

float calcMass(long time){
	return slope * time + interjection; //calculate mass of fuel based on time
}

float injectionRun(double RPM, float potentiometer) { //takes us through every step described above
	stop(); //make sure that we are not injecting
	uint32_t time = findTime(RPM, potentiometer); 
	start(); //start injection
	int stay = 1;
	long actualTime;
	uint32_t startTime = micros();
	while (stay) { //wait for time, made so that it can be interrupted
		if (time <= (micros() - startTime)) {
			actualTime = micros() - startTime;
			stay = 0;
		}
		else {
			stay = stay;
		}
	}
	stop(); //stop injection
	return calcMass(actualTime); //do the math
}