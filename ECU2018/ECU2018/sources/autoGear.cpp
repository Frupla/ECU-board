
#include "autoGear.h"
#include "rs232.h"
#include "global.h"



void autoGear() {
	//static uint8_t threshhold_down = 0;
	uint8_t gear = rio_rx[RIO_RX_GEAR];
	uint16_t rpm = (rio_rx[RIO_RX_RPM_H] << 8) | (rio_rx[RIO_RX_RPM_L]);

	if (rpm > 3800 && gear == 1 && rpm < 20000) {
		rio_tx[RIO_TX_GEAR_UP] = 1;
		rio_tx[RIO_TX_GEAR_DOWN] = 0;
		rio_tx[RIO_TX_NEUTRAL] = 0;
	}
	else {
		rio_tx[RIO_TX_GEAR_UP] = 0;
	}

	/*if (speed < 13 && gear == 2 && !rio_tx[RIO_TX_BURN]) {
		threshhold_down++;
		if (threshhold_down > THRESHHOLD) {
			rio_tx[RIO_TX_GEAR_DOWN] = 1;
			rio_tx[RIO_TX_GEAR_UP] = 0;
		}
	}
	else {
		threshhold_down = 0;
	}*/
}
