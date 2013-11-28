#include "PWMChip.h"

//  0   1   2   3   4   5   6   7
//0x01  02  04  08  10  20  40  80

typedef enum {kBit0=0x01,
    kBit1=0x02,
    kBit2=0x04,
    kBit3=0x08,
    kBit4=0x10,
    kBit5=0x20,
    kBit6=0x40,
    kBit7=0x80} bitmask;

const int PWM_FULL = -1;

const uint8_t REGBANK_OFFSET = 0x06;

// registers for the i2c
const uint8_t PRE_SCALE = 0xFE;

const uint8_t ALL_LED_START = 0xFA;
const uint8_t LED_BANK_START = 0x06;

const uint8_t MODE_1 = 0x00;
const uint8_t MODE_2 = 0x01;

const uint8_t MASK_FULL = kBit4; // bit 4

const uint8_t MASK_SLEEP = kBit4; // bit 4
const uint8_t MASK_OUTDRV = kBit2; // bit 2

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

void PWMChip::setAllChannels(float ontime) {
    if (ontime >= 1.0f) {
        writeChannel(channel, PWM_FULL, 0);
    } else if (ontime <= 0.0f) {
        writeChannel(channel, 0, PWM_FULL);
    } else {
        writeChannel(channel, 0, (int) (ontime * 4096.0));
    }
}

void PWMChip::setChannel(int channel, float ontime) {
	if (channel < 0 || channel > 15) {
		printf("Channel out of range 0-15; given %i.\n", channel);
		return;
	}
	int highstart, lowstart;

	if (ontime >= 1.0f) {
        highstart = PWM_FULL;
        lowstart = 0;
	} else if (ontime <= 0.0f) {
        highstart = 0;
        lowstart = PWM_FULL;
	} else {
        highstart = 0;
        lowstart = (int) (ontime * 4096.0);
	}
	
    writeSubChannel(ALL_LED_START, (highstart == PWM_FULL), highstart);
    writeSubChannel(ALL_LED_START+2, (lowstart == PWM_FULL), lowstart);
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
    printf("Reg %x data %x ret %c\n", reg, data, value ? 'Y' : 'N');
}

void PWMChip::writeChannel(uint8_t channel, int highstart, int lowstart) {
    uint8_t* reg1 = subchannel * 2 + REGBANK_OFFSET;
    writeSubChannel(reg1, (highstart == PWM_FULL), highstart);
    writeSubChannel(reg1+2, (lowstart == PWM_FULL), lowstart);
}

// only 12 lsb of period matters.
// if not enabled
void PWMChip::writeSubChannel(uint8_t register_start, bool full, uint32_t period) {
	uint32_t p = period & 0x00000FFF; // pattern 0x00000abc
	// do math as per pdf here!
	uint8_t reg = register_start;
	uint8_t low8 = (uint8_t) (p); // pattern 0xbc
	uint8_t high4 = (uint8_t) (p >> 8); // pattern 0x0a
    uint8_t high8 = high4 & (~MASK_FULL);
    
	if (full) {
		if (pwm_bank->Write(reg, MASK_FULL)) {
			printf("I2C %3d: Failed to write to subchannel %d\n", address, subchannel);
			return;
		}
	} else {
        if (pwm_bank->Write(reg, high8) || pwm_bank->Write(reg + 1, low8)) {
			printf("I2C %3d: Failed to write to subchannel %d\n", address, subchannel);
		}
	}
}

void PWMChip::getSubChannel(uint8_t subchannel, bool &full, int &start) {
	uint8_t high, low, reg;
	reg = subchannel * 2 + REGBANK_OFFSET;
	if (pwm_bank->Read(reg, 1, &high) || pwm_bank->Read(reg+1, 1, &low)) {
		printf("I2C %3d: Failed to read subchannel bytes\n", address);
        return;
	}
	uint32_t s = (((uint32_t)(high << 4)) << 4) | ((uint32_t)low);
    printf("Subchannel conv: H%x L%x -> %x\n", high, low, s);
	start = (int) s;
	full = (MASK_FULL & high);
}
