/*
 * rs232.cpp
 *
 * Created: 07/04/2017
 * Author: Håkon Westh-Hansen
 *
 */

/*===========*/
/* TODO list */
/*===========*/

/*
(1)
Make timeout for the flush, i.e. void rs232_wait_transmit()
*/

/*==========*/
/* Includes */
/*==========*/

#include "global.h"
#include "rs232.h"
#include "rs232sync.h"

/*=============*/
/* Definitions */
/*=============*/

#define BAUD232 19200
#define RS232SERIAL Serial3
#define RS232_TX_WAIT_TIMEOUT_CK (F_CPU / 8)

/*===========*/
/* Functions */
/*===========*/

void rs232_init() {
	RS232SERIAL.begin(BAUD232, SERIAL_8N1);
}

void rs232_putc(uint8_t data) {
	/* Wait for empty transmit buffer */
	rs232_wait_transmit(); // TODO check if we need this
	/* Send data */
	RS232SERIAL.write(data);
}

char rs232_getc() {
	return RS232SERIAL.read();
}

/* This might be dumb as hell, but the communication is not working :( */
uint8_t rs232_available() {
	if (RS232SERIAL.available() >= 1) {
		return 1;
	}
	return 0;
}

/* Checks if the transmitted message has been sent
TODO: Make timeout for the flush */
void rs232_wait_transmit() {
	RS232SERIAL.flush();
}
