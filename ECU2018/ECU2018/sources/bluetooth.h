/*
 * bluetooth.h
 *
 * Author: Håkon Westh-Hansen
 * Created: 1/4/2017
 *
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#define ANDROID_START_STRING "START"
#define ANDROID_END_STRING "END"

#define BLUE_SEND_BUFFER_SIZE 400

 /*=============*/
 /* Definitions */
 /*=============*/

#define BAUDBLUE 9600
#define BLUESERIAL Serial1

 /*=============*/
 /* Prototypes  */
 /*=============*/

void blue_init();
void blue_putc(unsigned char data);
void blue_puts(String s);
void blue_putsln(String s);
uint16_t blue_available();
void blue_flush_buffer();
unsigned char blue_read_data();
void blue_read_buffer(void * buff, uint16_t len);
void blue_set_new_device(String name);

#endif /* BLUETOOTH_H_ */
