/*
 * rs232sync.c
 *
 * Created: 21/06/2016 21:32:15
 * Author: Henning
 *
 */

/*==========*/
/* Includes */
/*==========*/

#include "canbus.h"
#include "inout.h"
#include "global.h"
#include "LEDs.h"
#include "rs232.h"
#include "rs232sync.h"

/*=============*/
/* Definitions */
/*=============*/

#define RS232_START_1	0x55
#define RS232_START_2	0xAA

/*========================*/
/* Variables & Prototypes */
/*========================*/

// Evil volatile variables
static volatile uint8_t _inTempData[RIO_RX_REGSIZE + 20];
static volatile uint8_t state = RS232_STATE_WAITING;
static volatile uint8_t dataIndex = 0;
static volatile uint8_t packetSize = 0;
static volatile bool isRioAlive;

static uint8_t car;

/*===========*/
/* Functions */
/*===========*/

/* Set the car ID */
void rs232_set_car(uint8_t carId) {
	car = carId;
}

/******************************************************************
 * Function: rs232_tx
 * Description: Syncs and sends data to the RIO
 * Inputs: "Data kept in rio_tx[] array"
 * Outputs: N/A
 ******************************************************************/
void rs232_tx() {
	rs232_putc(RS232_START_1);				// Sync
	rs232_putc(RS232_START_2);				// Sync
	rs232_putc(RIO_TX_REGSIZE);				// Length
	for(unsigned char i = 0; i < RIO_TX_REGSIZE; i++) {
		rs232_putc(rio_tx[i]);					// Data
	}
}

/******************************************************************
 * Function: _rs232_react
 * Description: Reads received data and performs actions based on it
 * Inputs: N/A
 * Outputs: "Sets isRioAlive true if RIO is still alive"
 ******************************************************************/
static inline void _rs232_react() {
	// Receive starter signal from RIO
	if (!CAN.goneToShit(STEERING_ID) && rio_rx[RIO_RX_STARTER] == 1 && rio_rx[RIO_RX_ALIVE] == 1 && !emergency) {  //
		setStarterHigh();
		LED_on(LED4);
	} else {
		setStarterLow();
		LED_off(LED4);
	}
	// Check if RIO is alive
	isRioAlive = (rio_rx[RIO_RX_ALIVE] == 1);
}

/******************************************************************
 * Function: _update_rs232_data
 * Description: updates recieved data array rio_rx[]
 * Inputs: N/A
 * Outputs: N/A
 ******************************************************************/
static inline void _update_rs232_data() {
	// When receiving data from RS232 RIO
	for(uint8_t i = 0; i < packetSize; i++) {
		rio_rx[i] = _inTempData[i];
	}
	_rs232_react();
}

/******************************************************************
 * Function: rs232_autoRecieve
 * Description: Handles all the communication with the RIO
 * Inputs: N/A
 * Outputs: N/A
 ******************************************************************/
void rs232_autoRecieve(unsigned char data) {
	switch(state){
		case RS232_STATE_WAITING:
		{
			if(data == RS232_START_1) {
				state = RS232_STATE_WAITING_FOR_SECOND_START;
			}
		}
		break;
		case RS232_STATE_WAITING_FOR_SECOND_START:
		{
			if(data == RS232_START_2) {
				state = RS232_STATE_GET_PACKET_SIZE;
			}
			else {
				state = RS232_STATE_WAITING;
			}
		}
		break;
		case RS232_STATE_GET_PACKET_SIZE:
		{
			packetSize = data;
			dataIndex = 0;
			state = RS232_STATE_RECEIVING;
		}
		break;
		case RS232_STATE_RECEIVING: // Receiving data.
		{
			if(dataIndex < packetSize) {
				_inTempData[dataIndex] = data;
				dataIndex++;
			}
			if(dataIndex == packetSize){
				_update_rs232_data();
				state = RS232_STATE_WAITING;
				LED_toggle(LED5);
			}
		}
		break;
	}
}

/* Interrupt handler for the RIO, basically a wrapper for rs232_autoRecieve */
void serialEvent3() {
	while (rs232_available()) {
		//Serial.print("serialEvent3 is running: "); TODO cleanup, might be good to have
		//unsigned char temp = rs232_getc();
		//Serial.println(temp);
		rs232_autoRecieve(rs232_getc());
	}
}
