#include "WPILib.h"

const int regbank_offset = 6;
const uint8_t ADDRESS = 0x00;// 

// registers for the i2c
const uint8_t PRE_SCALE = 0xFE;

const uint8_t ALL_LED_START = 0xFA;
const uint8_t LED_BANK_START = 0x06;

const uint8_t MODE_1 = 0x00;
const uint8_t MASK_SLEEP = 0x10;

const int PWM_FULL = -1;

class I_TOO_SEE_YOU: public SimpleRobot {
	I2C* pwm_bank;
public:
	I_TOO_SEE_YOU(void) {
		DigitalModule* mod = DigitalModule::GetInstance(1);
		pwm_bank = mod->GetI2C(ADDRESS);
	}
	
	// takes a value in Hz
	/**
	 * Takes a value in Hz. Wakes up the I2C
	 */
	void setPreScale(float update_rate) {
		const float OSC_CLOCK = 25000000;// 25 MHz
		uint8_t val = (uint8_t)(float)(OSC_CLOCK / (4096 * update_rate) - 1.0);
		

		setSleep(true);
		pwm_bank->Write(PRE_SCALE, val);
		setSleep(false);
	}
	
	void setSleep(bool asleep) {
		uint8_t s_mod1;
		pwm_bank->Read(MODE_1, 1, &s_mod1);
		if (asleep) {
			// set sleep bit high
			s_mod1 |= MASK_SLEEP;
		} else {
			// set sleep bit low
			s_mod1 &= (~MASK_SLEEP);
		}
		pwm_bank->Write(MODE_1, s_mod1);
	}
	
	/**
	 * highstart and lowstart are the time per period in fractions
	 * of 4096 (range 0-4095) of the update period. If one is PWM_OFF (-1).
	 * Each cycle, at time lowstart, the channel drops low; at time highstart, the 
	 * channel goes high
	 * 
	 */
	void writePWMChannel(int channel, int highstart, int lowstart=0) {
		writeSubChannel(channel * 2, (highstart == PWM_FULL), highstart);	
		writeSubChannel(channel * 2 + 1, (lowstart == PWM_FULL), lowstart);
	}
	
	/**
	 * 
	 * only 12 lsb of period matters.
	 * if not enabled
	 */
	void writeSubChannel(uint8_t subchannel, bool full, uint32_t period) {
		uint32_t p = period & 0x00000FFF; // pattern 0x00000abc
		// do math as per pdf here!
		uint8_t reg = subchannel * 2;
		uint8_t low8 = (uint8_t)p; // pattern 0xbc
		uint8_t high4 = (uint8_t)(p >> 8); // pattern 0x0a
		
		if (full) {
			pwm_bank->Write(reg, 0x10);// sets the 4th highest bit to one
		} else {
			pwm_bank->Write(reg, high4);
			pwm_bank->Write(reg+1, low8);
		}
	}

	/**
	 * Drive left & right motors for 2 seconds then stop
	 */
	void Autonomous(void) {
		printf("AUTON\n");
	}

	/**
	 * Runs the motors with arcade steering. 
	 */
	void OperatorControl(void) {
		printf("OPCONT\n");
		while (IsOperatorControl() && IsEnabled()) {

			Wait(0.1);
		}
	}

	/**
	 * Runs during test mode
	 */
	void Test() {
		printf("TEST\n");
	}

	void Disabled() {
		printf("Disabled\n");
		// we change LED1 frequency
		int i=0;
		while (IsOperatorControl() && IsEnabled()) {
			switch (i) {
			case 0:
				writePWMChannel(0, PWM_FULL);
				break;
			case 1:
				writePWMChannel(0, 0);
				break;
			case 2:
				writePWMChannel(0, 2047);
				break;
			case 3:
				writePWMChannel(0, 1023);
				break;
			}

			Wait(0.005);
		}
	}
};

START_ROBOT_CLASS(I_TOO_SEE_YOU)
;

