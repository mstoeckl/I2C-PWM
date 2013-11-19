#ifndef __PWM_CHIPSIE_H_
#define __PWM_CHIPSIE_H_

#include "WPILib.h"

class PWMChip {
public:
	static const int PWM_FULL = -1;

	PWMChip(uint8_t module, uint8_t address);

	/** highstart and lowstart are the time per period in fractions
	 * of 4096 (range 0-4095) of the update period. If one is PWM_OFF (-1).
	 * Each cycle, at time lowstart, the channel drops low; at time highstart, the 
	 * channel goes high */
	void writeChannel(int channel, int highstart, int lowstart = 0);

	void setSleep(bool asleep);
	void setTotemPole(bool on);

	/** Takes a value in Hz. Wakes up the PWM chip
	 * Range is 23.8 Hz to 6104 Hz */
	void setPreScale(float update_rate);

	~PWMChip();
private:
	void writeSubChannel(uint8_t subchannel, bool full, uint32_t period);
	void setRegisterBit(uint8_t reg, uint8_t mask, bool high);
	I2C* pwm_bank;
};

#endif // __PWM_CHIPSIE_H_
