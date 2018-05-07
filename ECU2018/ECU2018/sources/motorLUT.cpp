// 
// 
// 

#include "motorLUT.h"

uint32_t ignitionArray[25] = {
	20,	20,	20,	20,	20,	20,	20,	25,	25,	25,	30,	30,	30,	30,	35,	35,	35,	35,	35,	40,	40,	40,	40,	40,	45 //sample values, should proberly be something else
};
uint32_t injectionArray[25] = {
	6000, 4967, 5214, 6000, 6000, 4932, 6000, 4867, 4526, 4597, 4592, 4536, 4365, 4289, 4421, 4446, 4384, 4306, 4460, 4528, 4500, 4500, 4500, 4500, 4500//sample values, choosen based on advanced calculations
};

INTERPOL_t calculateInterpolation(double rpm) { 
	double rpmDivided = rpm / 250;
	INTERPOL_t interpolation;
	interpolation.lower = (int)rpmDivided;
	interpolation.upper = interpolation.lower + 1;
	interpolation.increment = rpmDivided - interpolation.lower;
	return interpolation;
}