// 
// Asger (s163905) 
// 

#include "position.h"
#include <avr\io.h>
#include <avr\interrupt.h>
#include "injection.h"
#include "ignition.h"

u_int encoder_A, encoder_B, encoder_Z;

int calibration_variable;
int32_t encoder_position;
bool zPulseFlag;
double time_injection_is_active;

bool is_inj; //if it isn't inj it's ign

uint8_t encoder_pin_A_intern;
uint8_t encoder_pin_B_intern;
uint8_t encoder_pin_Z_intern;

uint32_t encoder_Z_time;
uint32_t encoder_Z_time_old;

// Decoder 1 er wheelsensor
// Decoder 2 er motor RPM
QuadDecode_t QuadDecode;

// Interrups service routine for compare interrupt på ftm2
void ftm2_isr(void) {
	// Goal: Turn on inj
	if (is_inj) {
		startInj();
		TeensyDelay::trigger(time_injection_is_active, INJECTION_CHANNEL); // time_injection_is_active is a global variable, 
																		   // and is the delay before the set function activates. 
		is_inj = false;
	}
	else {
		startIgnition();
		TeensyDelay::trigger(DWELL_TIME, IGNITION_CHANNEL);
		is_inj = true;
	}
}


int16_t encoderPositionEngine() { // TODO - find ud af om det her virker, design en test & udfoer
	if (QuadDecode.getCounter2() - calibration_variable  < 0) {
		return (QuadDecode.getCounter2() / 4) + 360 - calibration_variable; 
	}
	else {
		return (QuadDecode.getCounter2() / 4) - 360 - calibration_variable;
	}
	// Divide by 4, because the hardware encoder counts on change on both channels. (4 counts per pulse)
	// % by 720 for the calibration variable.
}

int16_t encoderPositionWheel() {
	return QuadDecode.getCounter1()/4;
	// Divide by 4, because the hardware encoder counts on change on both channels. (4 counts per pulse)
}

void resetEncoderWheel(int16_t reset_to) {
	QuadDecode.setCounter1(reset_to);
}

// code written all on our lonesome
int altEncoderErrorCheck() // Returnerer EXIT_FAILURE hvis der er forskydning mellem A og B
{
	if ((int)encoder_A - (int)encoder_B > 1 || (int)encoder_A - (int)encoder_B < 1) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
} // er det en god ide at returnerer det EXIT_FAILURE? stopper det ikke bare koden?

void encoderInterrupthandlerZ() {
	encoder_Z++;
	QuadDecode.setCounter2(0); // correcting so TDC2 is 0 // TODO: This feels wrong, checkup
	encoder_Z_time_old = encoder_Z_time;
	encoder_Z_time = micros();
	zPulseFlag = true;

	float local_RPM = encoderRPM();
	time_injection_is_active = 100000;// findTime_injection(local_RPM, 1);
	//int32_t degree_the_ignition_should_activate = ignition_time_angle(local_RPM);

	//Set Registers for output compare mode - for IRQ? - See ftm2_isr(void)
	FTM2_COMBINE = 0;	    // Reset value, make sure
	FTM2_C0SC = 0x10;	      // Bit 4 Channel Mode
	FTM2_C0V = 340 * 4;//Ticks //(360 - degree_the_ignition_should_activate); // Initial Compare Interrupt Value // Shot in the dark

	FTM2_C1SC = 0x10;
	FTM2_C1V = (360 + 20) * 4;//Ticks //startAngle_inj;

							//  Set channel interrupt
	FTM2_C0SC = 0x50;     // Enable Channel interrupt and Mode 
	FTM2_C1SC = 0x50;     // Enable Channel interrupt and Mode 

	is_inj = false;

}


void initializeEncoder(uint8_t encoder_pin_Z, int calib_var)
{
	// Set pinmode
	pinMode(encoder_pin_Z, INPUT);
	// Write to intern variables
	calibration_variable = calib_var;
	encoder_pin_Z_intern = encoder_pin_Z;
	// Reset intern variables
	encoder_Z = 0;
	encoder_Z_time = 0;
	encoder_Z_time_old = 0;
	zPulseFlag = false;
	// Attach interrupts
	attachInterrupt(digitalPinToInterrupt(encoder_pin_Z), encoderInterrupthandlerZ, RISING);
}

void altInitializeEncoder(uint8_t encoder_pin_A, uint8_t encoder_pin_B, uint8_t encoder_pin_Z, int calib_var)
{
	// Set pinmode
	pinMode(encoder_pin_A, INPUT);
	pinMode(encoder_pin_B, INPUT);
	pinMode(encoder_pin_Z, INPUT);
	// Write to intern variables
	calibration_variable = calib_var;
	encoder_pin_A_intern = encoder_pin_A;
	encoder_pin_B_intern = encoder_pin_B;
	encoder_pin_Z_intern = encoder_pin_Z;
	// Reset intern variables
	encoder_A = 0; encoder_B = 0; encoder_Z = 0;
	encoder_position = calib_var;
	encoder_Z_time = 0;
	encoder_Z_time_old = 0;
	zPulseFlag = false;
	// Attach interrupts
	attachInterrupt(digitalPinToInterrupt(encoder_pin_A), altEncoderInterrupthandlerA, RISING);
	attachInterrupt(digitalPinToInterrupt(encoder_pin_B), altEncoderInterrupthandlerB, RISING);
	attachInterrupt(digitalPinToInterrupt(encoder_pin_Z), altEncoderInterrupthandlerZ, RISING);
}

int altEncoderPosition_A()
{
	return encoder_A;
}

int altEncoderPosition_B()
{
	return encoder_B;
}

int altEncoderPosition_Z()
{
	return encoder_Z;
}

int32_t altEncoderPositionEngine()
{	
	return encoder_position;
}

float encoderRPM() { // Returns the RPM. Returns -1 if fail, so every ~70min and startup
	if (encoder_Z_time - encoder_Z_time_old <= 0) {
		return -1;
	}
	float output_RPM = (120000000 / (float)(encoder_Z_time - encoder_Z_time_old));
	// T = encoder_Z_time - encoder_Z_time_old // How many µs between 2 ticks
	// 60 s/min * 10e6 µs/s  /  T µs   =  60000000 / T  rounds/min
	return output_RPM;
}

void altEncoderInterrupthandlerA() {
	encoder_A++;
	if (digitalRead(encoder_pin_B_intern)) {
		encoder_position++;
	}
	else {
		encoder_position--;
	}

}

void altEncoderInterrupthandlerB() {
	encoder_B++;
}

void altEncoderInterrupthandlerZ() {
	encoder_Z++;
	encoder_A = 0;
	encoder_B = 0;
	encoder_position = calibration_variable; // correcting so TDC2 is 0
	encoder_Z_time_old = encoder_Z_time;
	encoder_Z_time = micros();
	zPulseFlag = true;
}


bool getzPulseFlag() {
	return zPulseFlag;
}

void setzPulseFlag(bool what_to_be) {
	zPulseFlag = what_to_be;
}



// TODO: errorchecking with the Encoder library, to be done regularily, or as a seperate "errorcheck" file