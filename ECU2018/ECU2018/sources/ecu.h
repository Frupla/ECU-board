#pragma once

#include "ecu.h"

#ifndef ECU_H_
#define ECU_H_

void initializeEcu();

void startEngine();
void stopEngine();
bool isEngineRunning();

void ecuTimerCallback();

void ignitionAndInjectionTimerHandler();

float calculateConsumedFuelMass(float injectionDurationTime);

float calculateInjectionDurationTime(float RPM, float potentiometer);

float calculateIgnitionStopAngle(float rpm);

float calculateDwellAngle(float rpm);

#endif