// 
// 
// 
// Antænd mig
#include "ignition.h"


#define IGNITION_PIN 21
#define IGNITION_CHANNEL 0
#define DWELL_TIME 3000	// [us]

void initializeIgnition() {
	digitalWriteFast(IGNITION_PIN, LOW);
	pinMode(IGNITION_PIN, OUTPUT);
}

double calculateIgnitionStopAngle(double rpm) {
	INTERPOL_t ignitionInterpol = calculateInterpolation(rpm);
	double xlow = ignitionArray[ignitionInterpol.lower];
	double xhigh = ignitionArray[ignitionInterpol.upper];
	double xinc = ignitionInterpol.increment;
	double ignitionStopAngle = (xhigh - xlow) * xinc + xlow;
	return ignitionStopAngle;
}

double calculateDwellAngle(double rpm) {
	return (360 * rpm * DWELL_TIME) / 60000000;
}

inline void startIgnition() {
	//digitalWriteFast(IGNITION_PIN, HIGH); //Sends signal to start ignition
}

inline void stopIgnition() {
	//digitalWriteFast(IGNITION_PIN, LOW); //Sends signal to stop ignition
}

bool ignitionCheck(int ignitionStartAngle, int currentAngle) {
	// Check if we've reached the start angle, where we start charging the coil
	
}


