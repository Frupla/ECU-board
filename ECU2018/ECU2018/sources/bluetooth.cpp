/*
 * bluetooth.cpp
 *
 * Author: Håkon Westh-Hansen
 * Created: 1/4/2017
 *
 */

/*==========*/
/* Includes */
/*==========*/

#include "Arduino.h"
#include "bluetooth.h"

/*===========*/
/* Functions */
/*===========*/

void blue_init() {
	BLUESERIAL.begin(BAUDBLUE);
}

void blue_putc(unsigned char data) {
	BLUESERIAL.print(data);
}

void blue_puts(String s) {
	BLUESERIAL.print(s);
}

void blue_putsln(String s) {
	BLUESERIAL.println(s);
}

uint16_t blue_available() {
	return BLUESERIAL.available();
}

/* Flushes receive buffer, flush() function is not the same! */
void blue_flush_buffer() {
  BLUESERIAL.clear();
}

unsigned char blue_read_data() {
	return BLUESERIAL.read();
}

/******************************************************************
 * Function: blue_read_buffer
 * Description: Fills a buffer with incoming data from bluetooth
 * Inputs: Address of buffer and length of the data stream
 * Outputs: N/A
 ******************************************************************/
void blue_read_buffer(void * buff, uint16_t len) {
	uint16_t i;
	for(i=0; i<len; i++) {
		((char*)buff)[i] = blue_read_data();
	}
}

/******************************************************************
 * Function: blue_set_new_device
 * Description: Gives the bluetooth module a new name. For this to
 *              work you need to set the LE friend in CMD mode.
 * Inputs: Desired name
 * Outputs: N/A
 ******************************************************************/
void blue_set_new_device(String name) {
	delay(10000);
	// set name
	blue_puts("AT+GAPDEVNAME=");
  blue_puts(name);
  blue_puts("\n\r");
	delay(1000);
	// max power
	blue_puts("AT+BLEPOWERLEVEL=4");
	delay(1000);
	// disable CTS
	blue_puts("AT+UARTFLOW=off");
	delay(1000);
	blue_puts("ATZ\n\r");
}
