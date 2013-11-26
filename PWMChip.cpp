#include "PWMChip.h"

const int PWM_FULL = -1;

const uint8_t REGBANK_OFFSET = 0x06;

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
	this->address = address;
	if (pwm_bank->AddressOnly()) {
		printf("I2C %3d: Cannot connect\n", address);
	}
}

PWMChip::~PWMChip() {
	delete pwm_bank;
}

void PWMChip::setChannel(int channel, float ontime) {
	if (channel < 0 || channel > 15) {
		printf("Channel out of range 0-15; given %i.\n", channel);
		return;
	}

	if (ontime >= 1.0f) {
		writeChannel(channel, PWM_FULL, 0);
	} else if (ontime <= 0.0f) {
		writeChannel(channel, 0, PWM_FULL);
	} else {
		writeChannel(channel, 0, (int) (ontime * 4096.0));
	}
}

void PWMChip::getChannel(int channel, bool &highfull, int &high, bool &lowfull, int &low) {
	getSubChannel(channel * 2, highfull, high);
	getSubChannel(channel * 2 + 1, lowfull, low);
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

	bool sleeping;
	getSleep(sleeping);
	if (!sleeping) {
		setSleep(true);
	}
	bool aborted = pwm_bank->Write(PRE_SCALE, val);
	if (aborted)
		printf("I2C %3d: Failed to write prescale byte\n", address);
	if (!sleeping) {
		setSleep(false);
	}
}

void PWMChip::setSleep(bool asleep) {
	setRegisterBit(MODE_1, MASK_SLEEP, asleep);
}

void PWMChip::getSleep(bool &asleep) {
	getRegisterBit(MODE_1, MASK_SLEEP, asleep);
}

void PWMChip::setTotemPole(bool on) {
	setRegisterBit(MODE_2, MASK_OUTDRV, on);
}

void PWMChip::setRegisterBit(uint8_t reg, uint8_t mask, bool high) {
	uint8_t data;
	if (pwm_bank->Read(reg, 1, &data)) {
		printf("I2C %3d: Failed to read byte\n", address);
		return;
	}
	if (high) {
		data |= mask;
	} else {
		data &= (~mask);
	}
	if (pwm_bank->Write(reg, data)) {
		printf("I2C %3d: Failed to write byte\n", address);
		return;
	}
}

void PWMChip::getRegisterBit(uint8_t reg, uint8_t mask, bool &value) {
	uint8_t data;
	if (pwm_bank->Read(reg, 1, &data)) {
		printf("I2C %3d: Failed to read bit\n", address);
		return;
	}
	value = (data & mask);
}

void PWMChip::writeChannel(uint8_t channel, int highstart, int lowstart) {
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
		if (pwm_bank->Write(reg, MASK_FULL)) {
			printf("I2C %3d: Failed to write to subchannel %d\n", address, subchannel);
			return;
		}
	} else {
		if (pwm_bank->Write(reg, high4) || pwm_bank->Write(reg + 1, low8)) {
			printf("I2C %3d: Failed to write to subchannel %d\n", address, subchannel);
		}
	}
}

void PWMChip::getSubChannel(uint8_t subchannel, bool &full, int &start) {
	uint8_t high, low, reg;
	reg = subchannel * 2 + REGBANK_OFFSET;
	if (pwm_bank->Read(reg, 1, &high) || pwm_bank->Read(reg+1, 1, &low)) {
		printf("I2C %3d: Failed to read subchannel bytes\n", address);
	}
	uint32_t s = ((uint32_t)(high << 4) << 4) | ((uint32_t)low);
	start = (int) s;
	full = (MASK_FULL & high);
}
