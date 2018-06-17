#include "Arduino.h"
#include "ecu.h"
#include "position.h"
#include "motorLUT.h"
#include "TeensyDelay.h"
#include "inout.h"

#define IGNITION_DELAY_CHANNEL 0
#define INJECTION_DELAY_CHANNEL 1
#define INJECTION_PIN 20
#define IGNITION_PIN 21
// Max RPM
#define MAX_RPM 2500.0	// TODO Set to 4000
//Constants for the calculation of the mass of fuel based on time
#define SLOPE 0.78f
#define INTERJECTION 810.0f
#define DWELL_TIME_US 3000	// [us]
#define STARTER_RPM_CUTOFF 1100
// Gear values
#define FORCE_NEUTRAL 1 // TODO <- should be set by CAN steering wheel
#define GEAR_PWM_NEUTRAL 178	// TODO set to correct value
#define GEAR_PWM_FIRST	200	// TODO set to correct value
#define GEAR_PWM_SECOND	150	// TODO set to correct value
#define GEAR_UP_SPEED_THRESHOLD 18	  // [km/h]
#define GEAR_DOWN_SPEED_THRESHOLD 13  // [km/h]

#if DWELL_TIME_US > 9000
#error Dwell time too high (max 3ms)
#endif

void injectionDelayCallback();
void ignitionDelayCallback();

////////// ECU ////////////
// Timer
IntervalTimer ecuTimer;

// Fuel consumption
float consumedFuelMass = 0;
float potentiometer = 2;   // TODO: Make potentiometer variable: CAN.getMeasurement(RIO_POTENTIOMETER));

// Postion variables used for ignition and injection
int injectionStartAngle = 20;
float injectionDurationTime = 10;	// [us]
int ignitionStartAngle = 0;
int igntionStopAngle = 0;
volatile int currentAngle = 0;
uint32_t stopInjectionTime = 0;
uint32_t stopIgnitionTime = 0;

// More ignition and injection variables
volatile bool performIgnitionAndInjectionCalculationsFlag = false;
volatile bool ignitionFlag = false;		// Has ignition run since last Z-Pulse
volatile bool injectionFlag = false;	// Has injection run since last Z-Pulse
int dwellAngle = 0;
float RPM = 0;

bool engineRunning = false;

/////// ECU END ///////

void initializeEcu() {
	digitalWriteFast(INJECTION_PIN, LOW);
	digitalWriteFast(IGNITION_PIN, LOW);
	pinMode(INJECTION_PIN, OUTPUT);
	pinMode(IGNITION_PIN, OUTPUT);
	TeensyDelay::begin();
	TeensyDelay::addDelayChannel(injectionDelayCallback, INJECTION_DELAY_CHANNEL);
	TeensyDelay::addDelayChannel(ignitionDelayCallback, IGNITION_DELAY_CHANNEL);
}

void startEngine() {
	// Set up timer for injection and ignition
	ecuTimer.begin(ignitionAndInjectionTimerHandler, 10);
	ecuTimer.priority(1);
	// Set gear
	if(getSpeed() < GEAR_DOWN_SPEED_THRESHOLD) {
		if(FORCE_NEUTRAL) SetPWMDutyGear(GEAR_PWM_NEUTRAL);
		else SetPWMDutyGear(GEAR_PWM_FIRST);
	}
	// Run starter motor
	RPM = encoderRPM();
	if(RPM < STARTER_RPM_CUTOFF) {
		setStarterHigh();
	}
	engineRunning = true;
}

void stopEngine() {
	// Disable starter
	setStarterLow();
	// Stop timer
	ecuTimer.end();
	engineRunning = false;
}

bool isEngineRunning() {
	return engineRunning;
}

// Every 1 ms
void ecuTimerCallback() {
	// Ignition and Injection
	if(performIgnitionAndInjectionCalculationsFlag) {
		performIgnitionAndInjectionCalculationsFlag = false;
		RPM = encoderRPM();
		igntionStopAngle = -calculateIgnitionStopAngle(RPM);
		dwellAngle = calculateDwellAngle(RPM);
		ignitionStartAngle = igntionStopAngle - dwellAngle;
		injectionDurationTime = calculateInjectionDurationTime(RPM, potentiometer);
		consumedFuelMass += calculateConsumedFuelMass(injectionDurationTime);
	}
	if(engineRunning) {
		if(RPM > STARTER_RPM_CUTOFF) {
			setStarterLow();
		}
		if(getSpeed() < GEAR_DOWN_SPEED_THRESHOLD) {
			if(!FORCE_NEUTRAL) SetPWMDutyGear(GEAR_PWM_FIRST);
		} else if(getSpeed() > GEAR_UP_SPEED_THRESHOLD) {
			if(!FORCE_NEUTRAL) SetPWMDutyGear(GEAR_PWM_SECOND);
		}
	}
}

void injectionDelayCallback() {
	digitalWriteFast(INJECTION_PIN, LOW);
}

void ignitionDelayCallback() {
	digitalWriteFast(IGNITION_PIN, LOW);
}

// Interrupt handler takes about 1 us (TODO optimize)
inline void ignitionAndInjectionTimerHandler() {
	if(getZPulseFlag()) {
		performIgnitionAndInjectionCalculationsFlag = true;
		setZPulseFlag(false);
		ignitionFlag = true;
		injectionFlag = true;
	}
	if(injectionFlag && (RPM < MAX_RPM || RPM == -1)) {
		currentAngle = encoderPositionEngine();
		if(!digitalReadFast(INJECTION_PIN) && (currentAngle >= injectionStartAngle && currentAngle <= injectionStartAngle + 10)) {
			digitalWriteFast(INJECTION_PIN, HIGH);
			TeensyDelay::trigger(injectionDurationTime, INJECTION_DELAY_CHANNEL);
			injectionFlag = false;
		}
	}
	if(ignitionFlag && (RPM < MAX_RPM || RPM == -1)) {
		currentAngle = encoderPositionEngine();
		if(!digitalReadFast(IGNITION_PIN) && (currentAngle >= ignitionStartAngle && currentAngle <= ignitionStartAngle + 5)) {
			digitalWriteFast(IGNITION_PIN, HIGH);
			TeensyDelay::trigger(DWELL_TIME_US, IGNITION_DELAY_CHANNEL);
			ignitionFlag = false;
		}
	}
}

// Mass is calculated like this: m = slope * t + interjection
float calculateConsumedFuelMass(float injectionDurationTime) {
	return (SLOPE * injectionDurationTime + INTERJECTION) / (1000000.0f); // Calculate mass of fuel based on time, in grams
}

// Calculates injection duration in us
float calculateInjectionDurationTime(float RPM, float potentiometer) {
	INTERPOL_t injection = calculateInterpolation(RPM);
	float xhigh = injectionArray[injection.upper];
	float xlow = injectionArray[injection.lower];
	float xinc = injection.increment;
	float injectionTime = (xhigh - xlow) * xinc + xlow;
	return injectionTime * potentiometer;
}

float calculateIgnitionStopAngle(float rpm) {
	INTERPOL_t ignitionInterpol = calculateInterpolation(rpm);
	float xlow = ignitionArray[ignitionInterpol.lower];
	float xhigh = ignitionArray[ignitionInterpol.upper];
	float xinc = ignitionInterpol.increment;
	float ignitionStopAngle = (xhigh - xlow) * xinc + xlow;
	return ignitionStopAngle;
}

float calculateDwellAngle(float rpm) {
	return (360 * rpm * DWELL_TIME_US) / 60000000.0f;
}