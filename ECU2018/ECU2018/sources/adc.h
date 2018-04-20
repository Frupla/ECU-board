/*
 * adc.h
 *
 * Created: 17/5/2015 11:52:28
 * Author: Henning
 *
 */

#include "global.h"

#ifndef ADC_H_
#define ADC_H_

#define BATTERY_SENSOR 35
#define BRAKE_SENSOR 15

void adc_init();
// Perform a read on the ADC. For ADC 5, channel = 5.
uint16_t adc_read(uint8_t channel);
// Get the previous ADC4 (Battery voltage) value
uint16_t getBatteryAdcValue();
uint16_t getBrakeValue();
float getBatteryVoltage();

#endif /* ADC_H_ */
