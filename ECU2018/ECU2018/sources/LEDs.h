/*
 * LEDs.h
 *
 * Author: Håkon Westh-Hansen
 * Created: 1/4/2017
 *
 */

#ifndef LEDs_H_
#define LEDs_H_

#define LED1 24
#define LED2 25
#define LED3 26
#define LED4 27
#define LED5 29

void LED_init();
void LED_on(uint8_t LED);
void LED_off(uint8_t LED);
void LED_toggle(uint8_t LED);
void LED_test();

#endif /* LEDs_H_ */
