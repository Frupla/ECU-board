/*
 * global.h
 *
 * Global definitions for Motor 2017
 *
 * Created: 11/06/2016 17:37:30
 * Author: Henning
 *
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "Arduino.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

extern volatile uint8_t rio_tx[];
extern volatile uint8_t rio_rx[];

void global_init();

#endif /* GLOBAL_H_ */
