/*
 Name:		ECU2018.ino
 Created:	4/16/2018 9:52:09 AM
 Author:	Asger, Frederik, Irene og Berk
 Based on
 * MotorDynamo2017.ino
 *
 * Description: Main ino file for running the motorboard 2017
 * Author: Håkon Westh-Hansen
 * Created: 10/4/2017
 *
 */


/*==========*/
/* Includes */
/*==========*/

#include "injection.h"
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
#include "position.h"

/*=============*/
/* Definitions */
/*=============*/

#define A_PULSE 29 //A pulse
#define B_PULSE 30 //B pulse
#define Z_PULSE 28 //Z pulse
#define CANBUS_TX 33
#define CANBUS_RX 34

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
#define BUZZER_PIN 2


//for party
#define PARTY_ANIMATION_SPEED 400
//#define PARTY1 lol //pin
//#define PARTY2 lol //pin

/*========================*/
/* Variables & Prototypes */
/*========================*/

// Prototype Functions
static inline void blue_tx();
static inline void measureBatteryVoltage();
void partyAnimation(void);
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
int encoder_calibration_variable = 0; // Is set from labview


//emergency
uint8_t emergency = 0;


//wheelsensor 2.0
#define WHEEL_SENSOR_PIN_V_2 34
#define speedUpdate 250 //update period
uint32_t frontSpeedUpdate = 0;
volatile uint32_t wheelcountv2 = 0;
#define WHEEL_DIAMETER 0.5588f		// [m]
#define NUMBER_OF_EDGES_PER_REVOLUTION 30
#define WHEEL_CIRCUMFERENCE WHEEL_DIAMETER * PI

void setup() {
	/* Initializations */
	// Initialize USB communication
	Serial.begin(9600);
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

	//wheelsensor
	attachInterrupt(digitalPinToInterrupt(WHEEL_SENSOR_PIN_V_2), ISR_WHEEL, CHANGE);
	//Encoder
	initializeEncoder(A_PULSE, B_PULSE, Z_PULSE, encoder_calibration_variable);

	// Enable global interrupts
	sei();

	/* Play a song so we know we have started */
	sing(STARTUP_MELODY_ID);
	
	//Set up to read pulses
	pinMode(A_PULSE, INPUT);
	pinMode(B_PULSE, INPUT);
	pinMode(Z_PULSE, INPUT);

}

/*===========*/
/* Main loop */
/*===========*/
uint32_t speedTiming = 0;
float lastDist = 0;

// the loop function runs over and over again until power down or reset
void loop() {
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

		if (emergency) { //TODO emergency
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

	if (emergency) {
		if (tunes_is_ready()) {
			sing(GAME_MELODY_ID);
		}
	}

	if (CAN.getSystemState(STEERING_ID, BUTTON_PARTY)) {
		if (tunes_is_ready())
		{
			sing(SAX_MELODY_ID);
			//partyAnimation();
		}
	}

	LED_toggle(LED3);
}

// Herfra og ned, skamløst kopieret fra motorboard 2017:
//Author: Håkon Westh - Hansen
// Created : 10 / 4 / 2017
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
			LED_on(LED5);
			emergency = 1;
		}
	}
	else {
		miniVoltageTooLow = 0;
		rio_tx[RIO_TX_STOP] = 0;
		LED_off(LED5);
	}
}

void ISR_WHEEL() {
	wheelcountv2++;
}

float getDistancev2() {
	return (wheelcountv2 * WHEEL_CIRCUMFERENCE) / NUMBER_OF_EDGES_PER_REVOLUTION;
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