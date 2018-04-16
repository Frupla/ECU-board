/*
 * motorCAN.cpp
 *
 * Description: Fixes all the data that is send from/to
 * the motor board
 * Author: Håkon Westh-Hansen
 * Created: 21/4/2017
 *
 */

 /*==========*/
 /* Includes */
 /*==========*/

#include "adc.h"
#include "canbus.h"
#include "global.h"
#include "rs232.h"
#include "inout.h"
#include "motorCAN.h"
#include "autoGear.h"

/*===========*/
/* Functions */
/*===========*/

/******************************************************************
 * Function: _rs232_to_CAN_sync
 * Description: Sends/syncs received data from RIO with CAN
 * Inputs: N/A
 * Outputs: N/A
 ******************************************************************/
static inline void _rs232_to_CAN_sync() {
	// Update CAN measurements array

	// Motor / rio state variable. enum defined in canbus.
	uint32_t systemstate = 0;

	if (rio_rx[RIO_RX_ALIVE]) {
		systemstate |= 1 << STATE_RIO_ALIVE;
	}
	else {
		systemstate &= ~(1 << STATE_RIO_ALIVE);
	}

	if (rio_rx[RIO_RX_STARTER]) {
		systemstate |= 1 << STATE_RIO_STARTER;
	}
	else {
		systemstate &= ~(1 << STATE_RIO_STARTER);
	}

	CAN.sendStatus(63, systemstate);
	
	// TODO hot fix for world championship (2017 race)
	CAN.sendMeasurement(63, 0, RIO_REMAINING_FUEL, ((rio_rx[RIO_RX_REMAINING_FUEL_H] << 8) | rio_rx[RIO_RX_REMAINING_FUEL_L]) / 10.0 );

	CAN.sendMeasurement(63, 0, RIO_KMH,	rio_rx[RIO_RX_KMH]);
	//CAN.sendMeasurement(63, 0, MOTOR_SPEED, rio_rx[RIO_RX_KMH]); // The Motor Speed measurement is carried out locally
	CAN.sendMeasurement(63, 0, RIO_WATER_TEMP, rio_rx[RIO_RX_WATER_TEMP]);
	CAN.sendMeasurement(63, 0, RIO_OIL_TEMP, rio_rx[RIO_RX_OIL_TEMP]);
	CAN.sendMeasurement(63, 0, RIO_RPM, (rio_rx[RIO_RX_RPM_H] << 8) | rio_rx[RIO_RX_RPM_L]);
	CAN.sendMeasurement(63, 0, RIO_LAMBDA, rio_rx[RIO_RX_LAMBDA] / 100.0);
	CAN.sendMeasurement(63, 0, RIO_GEAR, rio_rx[RIO_RX_GEAR]);
	CAN.sendMeasurement(63, 0, RIO_KMPERL, (rio_rx[RIO_RX_KMPERL_H] << 8) | rio_rx[RIO_RX_KMPERL_L]);
	//CAN.sendMeasurement(63, 0, RIO_REMAINING_FUEL, (rio_rx[RIO_RX_REMAINING_FUEL_H] << 8) | rio_rx[RIO_RX_REMAINING_FUEL_L]);
}

/******************************************************************
 * Function: _CAN_transmit
 * Description: Sends data from the motor board out on the CAN bus
 * Inputs: N/A
 * Outputs: N/A
 * TODO -> speed, current, total power, total distance
 ******************************************************************/
static inline void _CAN_transmit() {
	//motor measurements
	CAN.sendMeasurement(63, 0, MOTOR_BRAKE, getBrakeValue());
	CAN.sendMeasurement(63, 0, MOTOR_BAT_VOLTAGE, getBatteryVoltage());
	CAN.sendMeasurement(63, 0, MOTOR_SPEED, 2*3.6f * getSpeed());
	CAN.sendMeasurement(63, 0, MOTOR_DISTANCE, getDistance());
}

/******************************************************************
 * Function: _rs232_to_CAN_sync
 * Description: Sends/syncs received data from motor/CAN to RIO
 * Inputs: N/A
 * Outputs: N/A
 ******************************************************************/
static inline void _CAN_to_rs232_sync() {
	// security precaution
	if (CAN.goneToShit(STEERING_ID) || emergency) {
		rio_tx[RIO_TX_BURN] = 0;	// send burn stop if steering has gone to shit, or the emergency button has been presed
	} else {
		if(CAN.getSystemState(STEERING_ID, BUTTON_HIGHBEAM) && CAN.getSystemState(AUTO_ID, AUTO_BURN)) {
			rio_tx[RIO_TX_BURN] = CAN.getSystemState(AUTO_ID, AUTO_BURN);	// "RIO_START"
		} else {
			rio_tx[RIO_TX_BURN] = CAN.getSystemState(STEERING_ID, BUTTON_BURN);	// "RIO_START"
		}
	}
	rio_tx[RIO_TX_IDLE] = CAN.getSystemState(STEERING_ID, BUTTON_IDLE);
	rio_tx[RIO_TX_CLOSED_LOOP] = CAN.getSystemState(STEERING_ID, BUTTON_CL);
	// rio_tx[RIO_TX_AUTOGEAR]	= CAN.getSystemState(STEERING_ID, BUTTON_AUTOGEAR);
	rio_tx[RIO_TX_AUTOGEAR] = 0; // TODO hotfix for Eco-marathon 2017
	if (CAN.getSystemState(STEERING_ID, BUTTON_AUTOGEAR)) {
		autoGear();
	}
	else {
		rio_tx[RIO_TX_GEAR_UP] = CAN.getSystemState(STEERING_ID, BUTTON_GEARUP);
	}
	rio_tx[RIO_TX_GEAR_DOWN] = CAN.getSystemState(STEERING_ID, BUTTON_GEARDOWN);
	rio_tx[RIO_TX_POT_UP]	= CAN.getSystemState(STEERING_ID, BUTTON_POTUP);
	rio_tx[RIO_TX_POT_DOWN] = CAN.getSystemState(STEERING_ID, BUTTON_POTDOWN);
	rio_tx[RIO_TX_NEUTRAL]	= CAN.getSystemState(STEERING_ID, BUTTON_NEUTRAL);
	rio_tx[RIO_TX_BRAKE]	= CAN.getSystemState(LIGHT_FRONT_ID, 0);
}

static inline void _motor_to_rs232_sync() {
	// Get wheel period from wheel sensor (interrupt based)
	uint16_t wheelPeriod = getWheelSensorPeriod();
	rio_tx[RIO_TX_WHEEL_PERIOD_H] = (wheelPeriod >> 8) & 0x00FF;
	rio_tx[RIO_TX_WHEEL_PERIOD_L] = wheelPeriod & 0x00FF;

	// Get distance completed
	uint16_t distance = getDistance();
	rio_tx[RIO_TX_DISTANCE_H] = (distance >> 8) & 0x00FF;
	rio_tx[RIO_TX_DISTANCE_L] = distance & 0x00FF;
}

static void _rs232_react() {
	// Set Gear
	uint16_t gearPwm = (((uint16_t) rio_rx[RIO_RX_SERVO_GEAR_H]) << 8) | rio_rx[RIO_RX_SERVO_GEAR_L];
	SetPWMDutyGear(gearPwm);
}

static inline void _CAN_debug() {

}

static inline void _RIO_debug() {
	// Header text so we understand what we are looking at
	Serial.print("RIO debug (RX):");
	Serial.print(" AL: ");
	Serial.print("\t WT: ");
	Serial.print("\t OT: ");
	Serial.print("\t RPM: ");
	Serial.print("\t LA: ");
	Serial.print("\t GR: ");
	Serial.print("\t KMH: ");
	Serial.print("\t KMPERL: ");
	Serial.print("\t REMF: ");
	Serial.print("\t START: ");
	Serial.println("\t SERVO: ");

	// Data from RIO -> MOTOR
	Serial.print("\t\t");
	Serial.print(rio_rx[RIO_RX_ALIVE]);
	Serial.print("\t ");
	Serial.print(rio_rx[RIO_RX_WATER_TEMP]);
	Serial.print("\t ");
	Serial.print(rio_rx[RIO_RX_OIL_TEMP]);
	Serial.print("\t ");
	Serial.print((uint16_t) rio_rx[RIO_RX_RPM_H] << 8 | rio_rx[RIO_RX_RPM_L]);
	Serial.print("\t ");
	Serial.print(rio_rx[RIO_RX_LAMBDA]);
	Serial.print("\t ");
	Serial.print(rio_rx[RIO_RX_GEAR]);
	Serial.print("\t ");
	Serial.print(rio_rx[RIO_RX_KMH]);
	Serial.print("\t ");
	Serial.print((uint16_t) rio_rx[RIO_RX_KMPERL_H] << 8 | rio_rx[RIO_RX_KMPERL_L]);
	Serial.print("\t\t ");
	Serial.print((uint16_t) rio_rx[RIO_RX_REMAINING_FUEL_H] << 8 | rio_rx[RIO_RX_REMAINING_FUEL_L]);
	Serial.print("\t ");
	Serial.print(rio_rx[RIO_RX_STARTER]);
	Serial.print("\t\t ");
	Serial.println((uint16_t) rio_rx[RIO_RX_SERVO_GEAR_H] << 8 | rio_rx[RIO_RX_SERVO_GEAR_L]);

	// What we are sending to RIO header
	Serial.print("RIO debug (TX):");
	Serial.print(" STOP: ");
	Serial.print("\t BURN: ");
	Serial.print("\t IDLE: ");
	Serial.print("\t CL: ");
	Serial.print("\t PTU: ");
	Serial.print("\t PTD: ");
	Serial.print("\t NEU: ");
	Serial.print("\t GRU: ");
	Serial.print("\t GRD: ");
	Serial.print("\t WH: ");
	Serial.print("\t WL: ");
	Serial.print("\t DH: ");
	Serial.print("\t DL: ");
	Serial.println("\t BRK: ");

	// Data from MOTOR -> RIO
	Serial.print("\t\t");
	Serial.print(rio_tx[RIO_TX_STOP]);
	Serial.print("\t ");
	Serial.print(rio_tx[RIO_TX_BURN]);
	Serial.print("\t ");
	Serial.print(rio_tx[RIO_TX_IDLE]);
	Serial.print("\t ");
	Serial.print(rio_tx[RIO_TX_CLOSED_LOOP]);
	Serial.print("\t ");
	Serial.print(rio_tx[RIO_TX_POT_UP]);
	Serial.print("\t ");
	Serial.print(rio_tx[RIO_TX_POT_DOWN]);
	Serial.print("\t ");
	Serial.print(rio_tx[RIO_TX_NEUTRAL]);
	Serial.print("\t ");
	Serial.print(rio_tx[RIO_TX_GEAR_UP]);
	Serial.print("\t ");
	Serial.print(rio_tx[RIO_TX_GEAR_DOWN]);
	Serial.print("\t ");
	Serial.print(rio_tx[RIO_TX_WHEEL_PERIOD_H]);
	Serial.print("\t ");
	Serial.print(rio_rx[RIO_TX_WHEEL_PERIOD_L]);
	Serial.print("\t ");
	Serial.print(rio_rx[RIO_TX_DISTANCE_H]);
	Serial.print("\t ");
	Serial.print(rio_rx[RIO_TX_DISTANCE_L]);
	Serial.print("\t ");
	Serial.println(rio_tx[RIO_TX_BRAKE]);
}

void motorCAN_sync() {
	// Send data RIO -> CAN bus
	_rs232_to_CAN_sync();
	// Send data motor -> CAN bus
	_CAN_to_rs232_sync();
	_CAN_transmit();
	// React on data
	_rs232_react();
	// Debugging
#ifdef RIO_DEBUG
	_RIO_debug();
#endif
}
