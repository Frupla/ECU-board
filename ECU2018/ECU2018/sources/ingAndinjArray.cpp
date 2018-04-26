// 
// 
// 

#include "ingAndinjArray.h"


uint32_t ignitionArray[25] = {
	20,	20,	20,	20,	20,	20,	20,	25,	25,	25,	30,	30,	30,	30,	35,	35,	35,	35,	35,	40,	40,	40,	40,	40,	45 //sample values, should proberly be something else
};
uint32_t injectionArray[25] = {
	6000, 4967, 5214, 6000, 6000, 4932, 6000, 4867, 4526, 4597, 4592, 4536, 4365, 4289, 4421, 4446, 4384,	4306, 4460, 4528, 4500,	4500, 4500, 4500, 4500//sample values, choosen based on advanced calculations
};
INTERPOL interpolation_map(double rpm) { 
	double RPM_DIVIDED = rpm / 250;
	INTERPOL INTERPOLATION;
	for (int i = 0; i <= 25; i++) {
		if (i >= RPM_DIVIDED) {
			INTERPOLATION.lower = i - 1;
			INTERPOLATION.upper = i;
			if (INTERPOLATION.upper - RPM_DIVIDED <= RPM_DIVIDED - INTERPOLATION.lower) {
				INTERPOLATION.close = INTERPOLATION.upper;
			}
			else {
				INTERPOLATION.close = INTERPOLATION.lower;
			}
			INTERPOLATION.increment = RPM_DIVIDED - INTERPOLATION.lower;
		}
	}
	return INTERPOLATION;
}