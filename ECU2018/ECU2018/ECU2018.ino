/*
 Name:		ECU2018.ino
 Created:	4/16/2018 9:52:09 AM
 Author:	Asger, Frederik, Irene og Berk
 Based on
 * MotorDynamo2017.ino
 *
 * Description: Main ino file for running the motorboard 2017
 * Author: H�kon Westh-Hansen
 * Created: 10/4/2017
 *
 */


/*==========*/
/* Includes */
/*==========*/

#include "sources/motorLUT.h"
#include "sources/ignition.h"
#include "sources/injection.h"
#include <canbus.h>
#include <tunes.h>
#include <ArduinoJson.h>
#include "sources/adc.h"
#include "sources/bluetooth.h"
#include "sources/global.h"
#include "sources/inout.h"
#include "sources/LEDs.h"
#include "sources/motorCAN.h"
#include "sources/rs232.h"
#include "sources/rs232sync.h"
#include "sources/position.h"
#include <Adafruit_SSD1306.h>


/*=============*/
/* Definitions */
/*=============*/

// Car ID
#define DTU_DYNAMO 1
#define CAR DTU_DYNAMO
// The number of 0 packets received from RIO before we determine the RIO is RIP
#define RIO_STOPPED_GRACE_VALUE 10
// The period of Bluetooth synchronization
#define BLUE_SYNC_PERIOD 1000
// The period of RIO synchronization
#define RS232_SYNC_PERIOD 100
// The period of RIO synchronization
#define CAN_SYNC_PERIOD 10
// Buzzer pin
#define BUZZER_PIN 36
// CAN pins
#define CANBUS_TX_PIN 33
#define CANBUS_RX_PIN 34
// OLED display reset pin
#define OLED_RESET_PIN 4

// For party
#define PARTY_ANIMATION_SPEED 400
//#define PARTY1 lol //pin
//#define PARTY2 lol //pin

/*========================*/
/* Variables & Prototypes */
/*========================*/

// Prototype Functions
static inline void blue_tx();
static inline void measureBatteryVoltage();
//void partyAnimation(void);
void ISR_WHEEL();
float getDistancev2();
float getSpeedv2();

// Counter for counts of too low measured Mini Voltage
static uint16_t miniVoltageTooLow = 0;
// Counter for received RIO Safe Shutdown signal or lost RIO connection
static uint16_t rioHasStopped = 0;
// Message to send to Android
//static char message[20];

/* Local variables for main loop */
uint32_t blueSyncTiming = 0;
uint32_t rs232SyncTiming = 0;
uint32_t CANSyncTiming = 0;

// Emergency
uint8_t emergency = 0;

// Wheelsensor 2.0
#define WHEEL_SENSOR_PIN_V_2 34
#define speedUpdate 250 //update period
uint32_t frontSpeedUpdate = 0;
volatile uint32_t wheelcountv2 = 0;
#define WHEEL_DIAMETER 0.5588f		// [m]
#define NUMBER_OF_EDGES_PER_REVOLUTION 30
#define WHEEL_CIRCUMFERENCE WHEEL_DIAMETER * PI
uint32_t speedTiming = 0;
float lastDist = 0;

////////// ECU ////////////
// Max RPM
#define MAX_RPM 4000.0

// Fuel consumption
float consumedFuelMass = 0;
float potentiometer = 2;

// Postion variables used for ignition and injection
int injectionStartAngle = 20;
double injectionDurationTime = 10;
int ignitionStartAngle = 0;
int igntionStopAngle = 0;
int currentAngle = 0;

// More ignition and injection variables
bool ignitionFlag = false;		// Has ignition run since last Z-Pulse
bool injectionFlag = false;	// Has injection run since last Z-Pulse
int dwellAngle = 0;
float RPM = 0;

/////// ECU END ///////

// Variables for tests
int lastRPMprint = 0;
uint32_t loopBeganAtMicros = 0;
uint32_t loopsSinceOutput = 0;
uint32_t timeAtLastDisplayOutput = 0;
uint32_t forMeasuringLoopTime = 0;

// Display 
Adafruit_SSD1306 display(OLED_RESET_PIN);
#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
B00000001, B11000000,
B00000001, B11000000,
B00000011, B11100000,
B11110011, B11100000,
B11111110, B11111000,
B01111110, B11111111,
B00110011, B10011111,
B00011111, B11111100,
B00001101, B01110000,
B00011011, B10100000,
B00111111, B11100000,
B00111111, B11110000,
B01111100, B11110000,
B01110000, B01110000,
B00000000, B00110000 };

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// SETUP //
void setup() {
	/* Initializations */
	// Initialize USB communication
	Serial.begin(9600);
	initializeIgnition();
	initializeInjection();
	// Initialize IO Pins
	io_init();
	// Initialize ADC
	adc_init();
	// Initialize LEDs
	LED_init();
	// Initialize PWM for Gear servo
	pwm_init();
	// Initialize Wheel Sensor
	wheel_sensor_init();
	// Initialize RS232
	rs232_init();
	rs232_set_car(CAR);
	// Initialize CAN
	can_init(MOTOR_ID);
	// Initialize Bluetooth
	blue_init();
	// Initialize Buzzer
	tunes_init(BUZZER_PIN);
	// Initialize display
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
	display.display();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.clearDisplay();
	display.setCursor(0, 0);

	// Set up timer for injection and ignition
	TeensyDelay::begin();

	// Wheelsensor
	//attachInterrupt(digitalPinToInterrupt(WHEEL_SENSOR_PIN_V_2), ISR_WHEEL, CHANGE);

	// Initialize Encoders
	int encoderTdcOffset = 0;	// TODO Get from somewhere
	initializeEncoder(encoderTdcOffset);

	// Enable global interrupts
	sei();

	// Play a song so we know we have started
	sing(STARTUP_MELODY_ID);
}

/*===========*/
/* Main loop */
/*===========*/
// The loop function runs over and over again until power down or reset
void loop() {
	loopBeganAtMicros = micros();
	//test for front wheelsensor
	/*if ((millis() - frontSpeedUpdate) > speedUpdate) {
	frontSpeedUpdate = millis();
	CAN.sendMeasurement(20, 0, MOTOR_SPEED, getSpeedv2());
	}*/

	//this is bullshit :) #quickfix	
	static uint32_t speedTiming = 0;
	if ((millis() - speedTiming) > 20) {
		speedTiming = millis();
		//speedSort();
	}


	// Annoy everyone until everything is restarted
	if (rioHasStopped > RIO_STOPPED_GRACE_VALUE) {
		if (tunes_is_ready()) {
			sing(SWEET_HOME_MELODY_ID);
		}
		rioHasStopped = 0;

	}

	// RS232 data is received in interrupt
	if (rio_rx[RIO_RX_ALIVE] != 1) {
		rioHasStopped++;
	}
	else {
		rioHasStopped = 0;
	}

	/* EMERGENCY STOP */

	// Read from Battery and external emergency stop sensor (VIN to Mini)
	measureBatteryVoltage();

	/* COMMUNICATION */

	// Transmit to CAN/update values
	if (millis() - CANSyncTiming >= CAN_SYNC_PERIOD) {
		// TODO Fix getspeed() -> Henning, Arek
		//getSpeed();		// [m/s]
		motorCAN_sync();
		//static uint16_t id = 0;
		//id++;
		//CAN.sendMeasurement(4,4,id, 1.21);

		if (emergency) { // TODO emergency
			CAN.emergencyStop(ERROR_EXTERNAL_BUTTON);
		}
		CANSyncTiming = millis();
	}

	// Transmit to RIO over RS232
	if (millis() - rs232SyncTiming >= RS232_SYNC_PERIOD) {
		rs232_tx();
		rs232SyncTiming = millis();
	}

	// Transmit to Bluetooth
	if (millis() - blueSyncTiming >= BLUE_SYNC_PERIOD) {
		blue_tx();
		blueSyncTiming = millis();
	}

	// Emergency detection
	if (emergency) {
		if (tunes_is_ready()) {
			sing(GAME_MELODY_ID);
		}
	}

	// Part Mode detection
	if (CAN.getSystemState(STEERING_ID, BUTTON_PARTY)) {
		if (tunes_is_ready())
		{
			sing(SAX_MELODY_ID);
			//partyAnimation();
		}
	}

	LED_toggle(LED3);

	// Ignition and Injection
	// Her pr�ver Frederik s� sm�t at tilf�je de nye funktioner, kommer det til at g� galt? Ja.
	currentAngle = encoderPositionEngine();
	RPM = encoderRPM();
	if(getZPulseFlag()) {
		igntionStopAngle = -calculateIgnitionStopAngle(RPM);
		dwellAngle = calculateDwellAngle(RPM);
		ignitionStartAngle =  igntionStopAngle - dwellAngle;
		injectionDurationTime = calculateInjectionDurationTime(RPM, potentiometer); // TODO: Make potentiometer variable: CAN.getMeasurement(RIO_POTENTIOMETER));
		consumedFuelMass += calculateConsumedFuelMass(injectionDurationTime);
		setZPulseFlag(false);
		ignitionFlag = true;
		injectionFlag = true;
	}
	if(injectionFlag && (RPM > MAX_RPM || RPM == -1)){
		injectionFlag = injectionCheck(injectionStartAngle, injectionDurationTime, currentAngle);
	}
	if(ignitionFlag && (RPM > MAX_RPM || RPM == -1)) {
		ignitionFlag = ignitionCheck(ignitionStartAngle, currentAngle);
	}
	
	// Debugging for Ignition and Injection
	loopsSinceOutput++;
	forMeasuringLoopTime += (micros() - loopBeganAtMicros);
	if (loopBeganAtMicros - timeAtLastDisplayOutput >= 100000) {
		Serial.print("Fuel burned: ");
		Serial.print(consumedFuelMass);
		Serial.print(" gram\n");
		timeAtLastDisplayOutput = loopBeganAtMicros;
		forMeasuringLoopTime /= loopsSinceOutput;
		display.print("loop time:  ");
		display.println(forMeasuringLoopTime);
		display.print("pos: ");
		display.println(currentAngle);
		display.print("inj stop: ");
		display.println((720*RPM*injectionDurationTime) / 60000000);
		forMeasuringLoopTime = 0;
		loopsSinceOutput = 0;
		display.display();
		display.clearDisplay();
		display.setCursor(0, 0);
	}
}

/*=========*/
/* Methods */
/*=========*/

static inline void blue_tx() {
	StaticJsonBuffer<BLUE_SEND_BUFFER_SIZE> sendBuffer;
	JsonObject& sendValue = sendBuffer.createObject();

	sendValue["tW"] = CAN.getMeasurement(RIO_WATER_TEMP);
	sendValue["tO"] = CAN.getMeasurement(RIO_OIL_TEMP);
	sendValue["bP1"] = CAN.getMeasurement(FRONT_BRAKE);
	sendValue["bP2"] = CAN.getMeasurement(MOTOR_BRAKE);
	sendValue["rR"] = CAN.getMeasurement(RIO_RPM);
	sendValue["lb"] = CAN.getMeasurement(RIO_LAMBDA);
	sendValue["sp"] = CAN.getMeasurement(RIO_KMH);	// or: CAN.getMeasurement(MOTOR_SPEED);
	sendValue["dW"] = getDistance();
	sendValue["gr"] = CAN.getMeasurement(RIO_GEAR);
	sendValue["bv"] = getBatteryVoltage();
	sendValue["rKL"] = CAN.getMeasurement(RIO_KMPERL);
	sendValue["bu"] = CAN.getSystemState(STEERING_ID, BUTTON_BURN);
	sendValue["rA"] = CAN.getSystemState(MOTOR_ID, STATE_RIO_ALIVE);
	sendValue["rS"] = CAN.getSystemState(MOTOR_ID, STATE_RIO_STARTER);
	sendValue["fC"] = CAN.getMeasurement(RIO_REMAINING_FUEL);
	//sendValue["msg"] = message;

	String blueSendBuffer;
	sendValue.printTo(blueSendBuffer);
	BLUESERIAL.print(blueSendBuffer);
}

static inline void measureBatteryVoltage() {
	uint16_t mini_voltage = adc_read(BATTERY_SENSOR);
	if (mini_voltage < 15000) { // TODO find a good value...
								// Emergency button must have been pressed
		miniVoltageTooLow++;
		if (miniVoltageTooLow > 5) {
			rio_tx[RIO_TX_STOP] = 1;
			LED_on(LED2);
			emergency = 1;
		}
	}
	else {
		miniVoltageTooLow = 0;
		rio_tx[RIO_TX_STOP] = 0;
		LED_off(LED2);
	}
}

void ISR_WHEEL() {
	wheelcountv2++;
}

float getDistancev2() {
	//return (wheelcountv2 * WHEEL_CIRCUMFERENCE) / NUMBER_OF_EDGES_PER_REVOLUTION;
	return ((encoderPositionWheel()/4) * WHEEL_CIRCUMFERENCE) / NUMBER_OF_EDGES_PER_REVOLUTION;
}
float getSpeedv2() {
	static float lastDist = 0;
	float delta = getDistancev2() - lastDist;
	lastDist = getDistancev2();
	float speed = (delta * 1000 * 3.6 / speedUpdate);
	Serial.print("delta: ");
	Serial.println(delta);
	Serial.print("speed: ");
	Serial.println(speed);

	return speed;
}