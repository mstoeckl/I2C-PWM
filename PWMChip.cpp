#include "PWMChip.h"

const int REGBANK_OFFSET = 6;

// registers for the i2c
const uint8_t PRE_SCALE = 0xFE;

const uint8_t ALL_LED_START = 0xFA;
const uint8_t LED_BANK_START = 0x06;

const uint8_t MODE_1 = 0x00;
const uint8_t MODE_2 = 0x01;

const uint8_t MASK_FULL = 0x20; // bit 4

const uint8_t MASK_SLEEP = 0x20; // bit 4
const uint8_t MASK_OUTDRV = 0x04; // bit 2

const float OSC_CLOCK = 25000000;// 25 MHz

PWMChip::PWMChip(uint8_t module, uint8_t address) {
	pwm_bank = DigitalModule::GetInstance(module)->GetI2C(address);
}

PWMChip::~PWMChip() {
	delete pwm_bank;
}

void PWMChip::setPreScale(float update_rate) {
	if (update_rate < 23.8) {
		printf(
				"Update rate too low, fundamentally capped below at 23.8 Hz. Given: %f Hz.\n",
				update_rate);
	} else if (update_rate > 6104.0) {
		printf(
				"Update rate too high, fundamentally capped above at 6104.0 Hz. Given: %f Hz.\n",
				update_rate);
	}
	uint8_t val = (uint8_t) (float) (OSC_CLOCK / (4096 * update_rate) - 1.0);

	setSleep(true);
	pwm_bank->Write(PRE_SCALE, val);
	setSleep(false);
}

void PWMChip::setSleep(bool asleep) {
	setRegisterBit(MODE_1, MASK_SLEEP, asleep);
}

void PWMChip::setTotemPole(bool on) {
	setRegisterBit(MODE_2, MASK_OUTDRV, on);
}

void PWMChip::setRegisterBit(uint8_t reg, uint8_t mask, bool high) {
	uint8_t s_mod1;
	pwm_bank->Read(reg, 1, &s_mod1);
	if (high) {
		s_mod1 |= mask;
	} else {
		s_mod1 &= (~mask);
	}
	pwm_bank->Write(reg, s_mod1);
}

void PWMChip::writeChannel(int channel, int highstart, int lowstart) {
	writeSubChannel(channel * 2, (highstart == PWM_FULL), highstart);
	writeSubChannel(channel * 2 + 1, (lowstart == PWM_FULL), lowstart);
}

// only 12 lsb of period matters.
// if not enabled
void PWMChip::writeSubChannel(uint8_t subchannel, bool full, uint32_t period) {
	uint32_t p = period & 0x00000FFF; // pattern 0x00000abc
	// do math as per pdf here!
	uint8_t reg = subchannel * 2 + REGBANK_OFFSET;
	uint8_t low8 = (uint8_t) (p & 0x000000FF); // pattern 0xbc
	uint8_t high4 = (uint8_t) (p >> 8); // pattern 0x0a

	if (full) {
		pwm_bank->Write(reg, MASK_FULL);// sets 4th bit
	} else {
		pwm_bank->Write(reg, high4);
		pwm_bank->Write(reg + 1, low8);
	}
}