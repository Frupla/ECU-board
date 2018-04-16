/*
 * global.c
 *
 * Created: 11/06/2016 17:52:00
 * Author: Henning
 *
 */

#include "global.h"
#include "rs232.h"

// RS232 Buffers
volatile uint8_t rio_tx[RIO_TX_REGSIZE] = {0}; // MOTOR -> RIO
volatile uint8_t rio_rx[RIO_RX_REGSIZE] = {0}; // RIO -> MOTOR

void global_init() {
	// Perform any needed initialization of the global variables here
}
