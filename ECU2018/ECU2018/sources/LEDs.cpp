/*
 * LEDs.cpp
 *
 * Author: Håkon Westh-Hansen
 * Created: 1/4/2017
 *
 */

/*==========*/
/* Includes */
/*==========*/

#include "global.h"
#include "LEDs.h"

/*===========*/
/* Functions */
/*===========*/

void LED_init() {
	// set the four LEDs as output pins
	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);
	pinMode(LED4, OUTPUT);
}

void LED_on(uint8_t LED) {
	digitalWriteFast(LED, HIGH);
}

void LED_off(uint8_t LED) {
	digitalWriteFast(LED, LOW);
}

void LED_toggle(uint8_t LED) {
	digitalWriteFast(LED, !digitalRead(LED));
}

/******************************************************************
 * Function: LED_test
 * Description: Toggles the on-board LED's in a nice little fashion
 * Inputs: N/A
 * Outputs: N/A
 ******************************************************************/
void LED_test() {
	LED_toggle(LED1);
	delay(500);
	LED_toggle(LED2);
	delay(500);
	LED_toggle(LED3);
	delay(500);
	LED_toggle(LED4);
	delay(500);
}
