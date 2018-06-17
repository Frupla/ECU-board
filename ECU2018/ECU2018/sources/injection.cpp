// Frederik Ettrup Larsen s163920
// 20/4/2018
// 

#include "injection.h"
#define INJECTION_CHANNEL 1
#define INJECTION_PIN 20
//Constants for the calculation of the mass of fuel based on time
#define SLOPE 0.78
#define INTERJECTION 810.0
// Mass i calculated like this: m = slope * t + interjection

void initializeInjection() {
	digitalWriteFast(INJECTION_PIN, LOW);
	pinMode(INJECTION_PIN, OUTPUT);
}

// Calculates injection duration in us
double calculateInjectionDurationTime(double RPM, float potentiometer) {
	INTERPOL_t injection = calculateInterpolation(RPM);
	double xhigh = injectionArray[injection.upper];
	double xlow = injectionArray[injection.lower];
	double xinc = injection.increment;
	double injectionTime = (xhigh - xlow) * xinc + xlow;
	return injectionTime * potentiometer;
}

inline void startInjection(){
	//digitalWriteFast(INJECTION_PIN, HIGH); // Sends signal to start injection
}

inline void stopInjection() {
	//digitalWriteFast(INJECTION_PIN, LOW); // Sends signal to stop injection
}

float calculateConsumedFuelMass(double injectionDurationTime) {
	return (SLOPE * injectionDurationTime + INTERJECTION)/(1000000); // Calculate mass of fuel based on time, in grams
}

bool injectionCheck(int injectionStartAngle, double  injectionDurationTime, int currentAngle) {

} 