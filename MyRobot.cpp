#include "WPILib.h"

const int regbank_offset = 6;
const uint8_t ADDRESS = 0x00;// 

// registers for the i2c
const uint8_t PRE_SCALE = 0xFE;

const uint8_t ALL_LED_START = 0xFA;
const uint8_t LED_BANK_START = 0x06;

const uint8_t MODE_1 = 0x00;
const uint8_t MODE_2 = 0x01;

const uint8_t MASK_FULL = 0x20; // bit 4

const uint8_t MASK_SLEEP = 0x20; // bit 4
const uint8_t MASK_OUTDRV = 0x04; // bit 2

const int PWM_FULL = -1;

class I_TOO_SEE_YOU: public SimpleRobot {
	I2C* pwm_bank;
public:
	/**
	 * Takes a value in Hz. Wakes up the PWM chip
	 * Range is 23.8 Hz to 6104 Hz
	 */
	void setPreScale(float update_rate) {
		if (update_rate < 23.8) {
			printf("Update rate too low, fundamentally capped below at 23.8 Hz. Given: %f Hz.\n", update_rate);
		} else if (update_rate > 6104.0) {
			printf("Update rate too high, fundamentally capped above at 6104.0 Hz. Given: %f Hz.\n", update_rate);
		}
		const float OSC_CLOCK = 25000000;// 25 MHz
		uint8_t val =
				(uint8_t) (float) (OSC_CLOCK / (4096 * update_rate) - 1.0);

		setSleep(true);
		pwm_bank->Write(PRE_SCALE, val);
		setSleep(false);
	}

	void setSleep(bool asleep) {
		setRegisterBit(MODE_1, MASK_SLEEP, asleep);
	}
	
	void setTotemPole(bool on) {
		setRegisterBit(MODE_2, MASK_OUTDRV, on);
	}

	void setRegisterBit(uint8_t reg, uint8_t mask, bool high) {
		uint8_t s_mod1;
		pwm_bank->Read(reg, 1, &s_mod1);
		if (high) {
			s_mod1 |= mask;
		} else {
			s_mod1 &= (~mask);
		}
		pwm_bank->Write(reg, s_mod1);
	}

	/**
	 * highstart and lowstart are the time per period in fractions
	 * of 4096 (range 0-4095) of the update period. If one is PWM_OFF (-1).
	 * Each cycle, at time lowstart, the channel drops low; at time highstart, the 
	 * channel goes high
	 * 
	 */
	void writePWMChannel(int channel, int highstart, int lowstart = 0) {
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
		uint8_t reg = subchannel * 2 + regbank_offset;
		uint8_t low8 = (uint8_t) (p & 0x000000FF); // pattern 0xbc
		uint8_t high4 = (uint8_t) (p >> 8); // pattern 0x0a

		if (full) {
			pwm_bank->Write(reg, MASK_FULL);// sets 4th bit
		} else {
			pwm_bank->Write(reg, high4);
			pwm_bank->Write(reg + 1, low8);
		}
	}

	I_TOO_SEE_YOU(void) {
		DigitalModule* mod = DigitalModule::GetInstance(1);
		pwm_bank = mod->GetI2C(ADDRESS);
		setTotemPole(true);
		setPreScale(100); // 100 Hz
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
		int i = 0;
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

