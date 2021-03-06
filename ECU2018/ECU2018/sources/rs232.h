﻿/*
 * rs232.h
 *
 * Created: 9/4/2017
 * Author:
 *
 */

#ifndef RS232_H_
#define RS232_H_

/* Main while loop waits for RS232 receive timeout [ms] or clock cycles */
#define RS232_RX_WAIT_TIMEOUT 10

/* Main while loop waits for RS232 transmit complete or timeout [clock cycles]
(warning, a value too low might corrupt transmit data) This is a worst case
timeout and if this happens, things have probably gone to shit anyway. */
#define RS232_TX_WAIT_TIMEOUT_CK (F_CPU / 8)

void rs232_init();
void rs232_putc(unsigned char data);
char rs232_getc();
void rs232_wait_transmit();
uint8_t rs232_available();

/* RIO TRANSMIT REGISTER (MOTOR->RIO) */
enum {
	RIO_TX_BURN,	// "RIO_START"
	RIO_TX_IDLE,
	RIO_TX_CLOSED_LOOP,
	RIO_TX_AUTOGEAR,
	RIO_TX_POT_UP,
	RIO_TX_POT_DOWN,
	RIO_TX_NEUTRAL,
	RIO_TX_GEAR_UP,
	RIO_TX_GEAR_DOWN,
	RIO_TX_STOP,
	RIO_TX_WHEEL_PERIOD_H,
	RIO_TX_WHEEL_PERIOD_L,
	RIO_TX_DISTANCE_H,
	RIO_TX_DISTANCE_L,
	RIO_TX_BRAKE,

	/*RIO_LAMBDA_H, // CURRENTLY NOT IN USE
	RIO_LAMBDA_L,*/

	// Leave this last entry untouched
	RIO_TX_REGSIZE
};

/* RIO RECEIVE REGISTER (RIO->MOTOR) */
enum {
	RIO_RX_ALIVE,
	RIO_RX_WATER_TEMP,
	RIO_RX_OIL_TEMP,
	RIO_RX_RPM_L,
	RIO_RX_RPM_H,
	RIO_RX_LAMBDA,		// Excess air ratio
	RIO_RX_GEAR,
	RIO_RX_KMH,			// Speed [km/h]
	RIO_RX_KMPERL_L,	// Fuel consumption [km/l]
	RIO_RX_KMPERL_H,	// Fuel consumption [km/l]
	RIO_RX_REMAINING_FUEL_L,
	RIO_RX_REMAINING_FUEL_H,
	RIO_RX_POTENTIOMETER,
	RIO_RX_STARTER,
	RIO_RX_SERVO_GEAR_L,
	RIO_RX_SERVO_GEAR_H,

	// Leave this last entry untouched
	RIO_RX_REGSIZE
};

#endif /* RS232_H_ */
