/*
 * adc.cpp
 *
 * Created: 17/5/2015 11:30:44
 * Author: Henning
 *
 */

/*==========*/
/* Includes */
/*==========*/

 #include "Arduino.h"
 #include "global.h"
 #include "adc.h"

/*===========*/
/* Functions */
/*===========*/

static uint16_t adcBatVolt = 0;

void adc_init() {
  // sets the resolution for analogRead
  analogReadResolution(16);
}

/******************************************************************
 * Function: adc_read
 * Description: Read from the analog to digital converter (ADC)
 * Inputs: ADC channel, e.g. for A5 channel = 5
 * Outputs: Digital value of the analog voltage read via the ADC
 ******************************************************************/
uint16_t adc_read(uint8_t channel) {
	uint16_t adc_value = analogRead(channel);
	if(channel == BATTERY_SENSOR) {
    adcBatVolt = adc_value;	// Save the value for future use if it is the battery voltage (DYNAMO)
  }
  return adc_value; // Returns the ADC value of the chosen channel
}

uint16_t getBrakeValue() {
  return adc_read(BRAKE_SENSOR);
}

uint16_t getBatteryAdcValue() {
  return adcBatVolt;
}

// Get the previous ADC23 (Battery voltage DYNAMO) value
float getBatteryVoltage() {
  float adcToVolt = 12.11 / 31610.0; // found experimentally
	return getBatteryAdcValue() * adcToVolt;
}
