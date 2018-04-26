/*
 * inout.cpp
 *
 * Author: H�kon Westh-Hansen
 * Created: 7/4/2017
 *
 */

 /*==========*/
 /* Includes */
 /*==========*/

#include "global.h"
#include "Arduino.h"
#include "adc.h"
#include "inout.h"
#include "LEDs.h"
#include "my_queue.h"

/*=============*/
/* Definitions */
/*=============*/

#define ONBOARD_LED 13
#define STARTER_PIN 6
#define HORN_PIN 7
#define GEAR_SENSOR_PIN 24
#define WHEEL_SENSOR_PIN_0 3	// A (FLOW, BOTTOM)
#define WHEEL_SENSOR_PIN_1 4	// B (WHEEL, TOP)
#define GEAR_SERVO_PIN 38
#define WHEEL_DIAMETER 0.5588f		// [m]
#define NUMBER_OF_EDGES_PER_REVOLUTION 60  // 120 when using both sensors
#define WHEEL_CIRCUMFERENCE WHEEL_DIAMETER * PI
#define TICK_DIFFERENCE_TOLERANCE (3 * WHEEL_CIRCUMFERENCE / NUMBER_OF_EDGES_PER_REVOLUTION)
#define CAPACITY 1
#define SPEED_TIME_TICK_DIFFERENCE_CAPACITY 30

/*========================*/
/* Variables & Prototypes */
/*========================*/

double lastDistance = 0;
elapsedMicros elapsedTimeSinceLastGetSpeed;
static volatile int32_t wheelEncoderPosition = 0;
static volatile uint8_t wheelState = 0;
static volatile uint32_t wheelCount = 0;    // Accumulate 32-bit int
static volatile uint16_t wheelPeriod = UINT16_MAX;  // Last measurement 16 bit for RIO (mu sec)
static volatile uint32_t wheelTime = 0;
static volatile uint64_t distanceCount = 0;    // Accumulate 64-bit int
static volatile uint64_t leftDistanceCount = 0;    // Accumulate 32-bit int
static volatile uint64_t rightDistanceCount = 0;    // Accumulate 32-bit int
static volatile double speed = 0;
MyQueue speedQueue(CAPACITY);
MyQueue timeDifferenceQueue(SPEED_TIME_TICK_DIFFERENCE_CAPACITY);
IntervalTimer myTimer;

double distance = 0;
double previousDistanceCount = 0;

volatile int queuePopulated = 0;
#define MEDIAN_MIDDLE_SIZE_PERCENTAGE 1.0f

unsigned long lastIsr0Time = 0;
unsigned long lastIsr1Time = 0;

uint8_t onboardLed = 0;

/*===========*/
/* Functions */
/*===========*/

inline void sort(float a[], int size) {
	for (int i = 0; i < (size - 1); i++) {
		for (int o = 0; o < (size - (i + 1)); o++) {
			if (a[o] > a[o + 1]) {
				float t = a[o];
				a[o] = a[o + 1];
				a[o + 1] = t;
			}
		}
	}
}

void io_init() {
	// Starter as output
	pinMode(STARTER_PIN, OUTPUT);
	setStarterLow();
	// Horn as output
	pinMode(HORN_PIN, OUTPUT);
	setHornLow();
	// Gear sensor as input
	pinMode(GEAR_SENSOR_PIN, INPUT);
	// Onboard LED
	pinMode(ONBOARD_LED, OUTPUT);
	myTimer.begin(timerCallback, 10000);
}

void ISR_WHEEL_0() {
	/*//Serial.print("0: ");
	//Serial.println(digitalReadFast(WHEEL_SENSOR_PIN_0), BIN);
	//Serial.print(micros());
	//Serial.print("\t");
	//Serial.println(digitalReadFast(WHEEL_SENSOR_PIN_0));
	wheelCount++;
	//leftDistanceCount++;
	distanceCount++;
	// Fix for losing ticks
	//if (leftDistanceCount < rightDistanceCount - TICK_DIFFERENCE_TOLERANCE) {
	//	leftDistanceCount = rightDistanceCount;
	//	distanceCount = 2 * rightDistanceCount;
	//}
	unsigned long isr0Elapsed = micros() - lastIsr0Time;
	lastIsr0Time = micros();
	timeDifferenceQueue.add((float)isr0Elapsed);
	queuePopulated++;
	Serial.println("ISR0");
	//isrWheel0Elapsed = 0;*/
}

void ISR_WHEEL_1() {
	//Serial.print("1: ");
	//Serial.println(digitalReadFast(WHEEL_SENSOR_PIN_1), BIN);
	//Serial.print(micros());
	//Serial.print("\t\t");
	//Serial.println(digitalReadFast(WHEEL_SENSOR_PIN_1));
	wheelCount++;
	//rightDistanceCount++;
	distanceCount++;
	// Fix for losing ticks
	/*if (rightDistanceCount < leftDistanceCount - TICK_DIFFERENCE_TOLERANCE) {
		rightDistanceCount = leftDistanceCount;
		distanceCount = 2 * leftDistanceCount;
	}*/
	unsigned long isr1Elapsed = micros() - lastIsr1Time;
	lastIsr1Time = micros();
	timeDifferenceQueue.add((float)isr1Elapsed);
	isr1Elapsed = 0;
	queuePopulated++;
	onboardLed = !onboardLed;
	digitalWriteFast(ONBOARD_LED, onboardLed);
}

void wheel_sensor_init() {
	pinMode(WHEEL_SENSOR_PIN_0, INPUT);
	pinMode(WHEEL_SENSOR_PIN_1, INPUT);
	attachInterrupt(digitalPinToInterrupt(WHEEL_SENSOR_PIN_0), ISR_WHEEL_0, CHANGE);
	attachInterrupt(digitalPinToInterrupt(WHEEL_SENSOR_PIN_1), ISR_WHEEL_1, CHANGE);
}

uint16_t getWheelSensorPeriod() {
	//rio expects microseconds according to comment in top ..
	return 0;	// TODO
}

// Every 10 ms
inline void timerCallback() {
	/*double distanceCountDifference = distanceCount - previousDistanceCount;
	if(fabsf(distanceCountDifference) > DISTANCE_COUNT_TOLERANCE) {
		distanceCount = previousDistanceCount;
	}*/
	distance = (double)distanceCount * WHEEL_CIRCUMFERENCE / (double)NUMBER_OF_EDGES_PER_REVOLUTION;
	//Serial.printf("DistanceCount: %lld\n", distanceCount);
	//Serial.printf(", Distance: %.4f\n", distance);

	speedSort();
	// TODO Test this
	if (micros() - lastIsr0Time > 1000000 && micros() - lastIsr1Time > 1000000) {
		//Serial.printf("Zero\n");
		speedQueue.add(0);
	}
}

/* Returns the speed taken from the distance measured since last call of getSpeed() in m/s */
// TODO check direction somehow
double getSpeed() {
	/*if(elapsedTimeSinceLastGetSpeed > 500000) {
		double distance = getDistance();
		speed = (distance - lastDistance) / elapsedTimeSinceLastGetSpeed * 1000000;
		elapsedTimeSinceLastGetSpeed = 0;
		Serial.printf("Speed: %.4f, DistanceDiff: %f\n", speed, (distance - lastDistance));
		lastDistance = distance;
	}
	return speed;*/
	float speed = speedQueue.getAverage();
	//Serial.printf("Speed: %f km/h\n", speed * 3.6f);
	//Serial.printf("DistanceCount: %d\n", distanceCount);
	//Serial.printf("%ld, %lld\n", millis(), distanceCount);
	return speed;
}

double getDistance() {
	return distance;
}
// END WHEEL SENSOR

void setHornHigh() {
	digitalWriteFast(HORN_PIN, HIGH);
}
void setHornLow() {
	digitalWriteFast(HORN_PIN, LOW);
}

void setStarterHigh() {
	digitalWriteFast(STARTER_PIN, HIGH);
}
void setStarterLow() {
	digitalWriteFast(STARTER_PIN, LOW);
}

uint8_t digitalReadGSensor() {
	return adc_read(GEAR_SENSOR_PIN);
}

// Sets the duty cycle of the PWM between 0 and 4095 (2016 value was 2499)
void SetPWMDutyGear(uint16_t duty) {
	float conversionFactor = 1.64; // conversion from 2016 code
	float convertedValue = duty * conversionFactor;
	analogWrite(GEAR_SERVO_PIN, (unsigned int)convertedValue);
}

// Initializes the PWM Output
void pwm_init() {
	pinMode(GEAR_SERVO_PIN, OUTPUT);
	analogWriteFrequency(GEAR_SERVO_PIN, 50); // set pwm frequency to 50Hz
	analogWriteResolution(12); // analogWrite value 0 to 4095, or 4096 for high
}

void speedSort() {
	// TODO Move code in this function below here to be called NOT from interrupt
	if (queuePopulated > SPEED_TIME_TICK_DIFFERENCE_CAPACITY) {
		float* timeDifferenceArray = timeDifferenceQueue.getArray();
		sort(timeDifferenceArray, SPEED_TIME_TICK_DIFFERENCE_CAPACITY);
		float sum = 0;
		int beginIndex = 0; // (SPEED_TIME_TICK_DIFFERENCE_CAPACITY - SPEED_TIME_TICK_DIFFERENCE_CAPACITY * MEDIAN_MIDDLE_SIZE_PERCENTAGE);
		int endIndex = SPEED_TIME_TICK_DIFFERENCE_CAPACITY; // -SPEED_TIME_TICK_DIFFERENCE_CAPACITY * MEDIAN_MIDDLE_SIZE_PERCENTAGE;
		for(int i = beginIndex; i < endIndex; i++) {
			sum += timeDifferenceArray[i];
		}
		//Serial.printf("Sum: %f\n", sum);
		float averageTimeDifference = sum / (SPEED_TIME_TICK_DIFFERENCE_CAPACITY * MEDIAN_MIDDLE_SIZE_PERCENTAGE);
		//Serial.printf("averageTimeDifference: %f\n", averageTimeDifference);
		float speed = WHEEL_CIRCUMFERENCE / (float)NUMBER_OF_EDGES_PER_REVOLUTION / averageTimeDifference * 1000000;	// [m/s]
		speedQueue.add(speed);
		queuePopulated = 0;
	}
}
