// Frederik Ettrup Larsen s163920
// 20/4/2018
// 

#include "injection.h"

#define inject_pin 20//Might be 20, if it is "dyse" which I'm assuming it is

int canInjectionRun(int RPM) {
	if (RPM > MAXRPM){
		return 0;
	}
	else {
		return 1;
	}
}

long findTime(float RPM, int potentiometer) {
	injection = Interpolation_map(RPM);
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
	return a * time + b; //calculate mass of fuel based on time
}

float injectionRun(float RPM, int potentiometer) { //takes us through every step described above
	if (canInjectionRun(RPM)) { //Only run if possible to run
		stop(); //make sure that we are not injecting
		long time = findTime(RPM, potentiometer); 
		start(); //start injection
		int stay = 1;
		long actualTime;
		long startTime = micros();
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
	else {
		return 0;
	}
}