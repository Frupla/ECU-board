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

#include "sources/ingAndinjArray.h"
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

#define A_PULSE 29 //A pulse
#define B_PULSE 30 //B pulse
#define Z_PULSE 28 //Z pulse
#define CANBUS_TX 33
#define CANBUS_RX 34
#define OLED_RESET 4


/* Pins jeg ikke ved noget om
adc.h -> Brake sensor pin    :    Har vi den stadig, og hvad laver den?
inout.cpp -> Horn_pin   :     Vi har ikke wiret et horn?
inout.cpp -> GEAR_SENSOR_PIN 24    :     No?


*/


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

//Fuel consumption
float fuelMass = 0;
float potentiometer = 2;
int startInjection = 0;

//Postion variables used for ign and inj
int startAngle_inj = 20;
double  time_inj = 10;
int startAngle_ign = 0;
char stopAngle_ign = 0;
int32_t posAngle = 0;

//More variables
float RPM = 0;

//variables for tests
int lastRPMprint = 0;
uint32_t loopBeganAtMicros = 0;
uint32_t loopsSinceOutput = 0;
uint32_t timeAtLastDisplayOutput = 0;
uint32_t forMeasuringLoopTime = 0;

// Display 
Adafruit_SSD1306 display(OLED_RESET);
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
	// Initialize encoder
	//altInitializeEncoder(A_PULSE, B_PULSE, Z_PULSE, encoder_calibration_variable);

	//Set up timer for injection and ignition
	TeensyDelay::begin();

	//wheelsensor
	//attachInterrupt(digitalPinToInterrupt(WHEEL_SENSOR_PIN_V_2), ISR_WHEEL, CHANGE);
	//Encoder
	initializeEncoder(Z_PULSE, encoder_calibration_variable);

	// Enable global interrupts
	sei();

	/* Play a song so we know we have started */
	sing(STARTUP_MELODY_ID);

}

/*===========*/
/* Main loop */
/*===========*/
uint32_t speedTiming = 0;
float lastDist = 0;
bool ignition_flag = false;
bool injection_flag = false;
int injDebug = 0;
int ignDebug = 0;
int ignAngle = 0;
int dwellAngle = 0;

// the loop function runs over and over again until power down or reset
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

	// Her prøver Frederik så småt at tilføje de nye funktioner, kommer det til at gå galt? Ja.

	//posAngle = encoderPositionEngine();
	RPM = encoderRPM();

	if (getzPulseFlag()) {
		//ignAngle = ignition_time_angle(RPM);
		//dwellAngle = ignition_dwell_angle(RPM);
		//startAngle_ign =  ignAngle - dwellAngle;
		//ignAngle = -5;
		//startAngle_ign = -20;
		//time_inj = findTime_injection(RPM, potentiometer);//CAN.getMeasurement(RIO_POTENTIOMETER));
		//time_inj = 100000;
		fuelMass = fuelMass + calcMass(time_inj);
		//Serial.print("Fuel burned: ");
		//Serial.print(fuelMass);
		//Serial.print(" units\n");
		setzPulseFlag(false);
		//ignition_flag = true;
		//injection_flag = true;
	}


	/*if (canRun(RPM) && injection_flag){
		injection_flag = injectionCheck(startAngle_inj, time_inj, posAngle);
		if (!injection_flag) {
			injDebug++;
		}
	}

	if (canRun(RPM) && ignition_flag) {
		ignition_flag = ignitionCheck(startAngle_ign, posAngle);
		if (!ignition_flag) {
			ignDebug++;
		}
	}*/
	
	loopsSinceOutput++;
	forMeasuringLoopTime += (micros() - loopBeganAtMicros);
	if (loopBeganAtMicros - timeAtLastDisplayOutput >= 100000) { // TODO: Fix if overflow
		timeAtLastDisplayOutput = loopBeganAtMicros;
		forMeasuringLoopTime /= loopsSinceOutput;
		//display.print("start angle:  ");
		display.println(startAngle_ign);
		display.print("Debug_variable: ");
		display.println(return_debug_variable());
		display.print("Z_pulses: ");
		display.println(EncoderPosition_Z());
		display.print("pos: ");
		display.println(posAngle);
		display.print("inj stop: ");
		display.println((720*RPM*time_inj) / 60000000);
		forMeasuringLoopTime = 0;
		loopsSinceOutput = 0;
		display.display();
		display.clearDisplay();
		display.setCursor(0, 0);
	}

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