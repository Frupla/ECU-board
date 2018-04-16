/*
 Name:		ECU2018.ino
 Created:	4/16/2018 9:52:09 AM
 Author:	Asger, Frederik, Irene og Berk
*/


#define EncoderA 29
#define EncoderB 30
#define setppulse 28
#define canbus_TX 33
#define canbus_RX 34

/*==========*/
/* Includes */
/*==========*/

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

// the setup function runs once when you press reset or power the board
void setup() {
	pinMode(EncoderA, INPUT);
	pinMode(EncoderB, INPUT);
	pinMode(setppulse, INPUT);

}

// the loop function runs over and over again until power down or reset
void loop() {

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