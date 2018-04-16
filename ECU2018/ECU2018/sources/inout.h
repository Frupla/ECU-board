/*
 * inout.h
 *
 * Author: Håkon Westh-Hansen
 * Created: 7/4/2017
 *
 */

#include "global.h"

#ifndef INOUT_H_
#define INOUT_H_

void io_init();
void wheel_sensor_init();
void setHornHigh();
void setHornLow();
void setStarterHigh();
void setStarterLow();
void SetPWMDutyGear(uint16_t duty);
void pwm_init();
uint8_t digitalReadGSensor();
uint16_t getWheelSensorPeriod();
inline void timerCallback();
double getDistance();
double getSpeed();
void speedSort();
#endif /* INOUT_H_ */
