// 
// 
// 

#include "ingAndinjArray.h"


uint32_t ignitionArray[25] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24 //sample values, should proberly be something else
};
uint32_t injectionArray[25] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24 //sample values, choosen based on advanced calculations
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