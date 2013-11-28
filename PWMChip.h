#ifndef __PWM_CHIPSIE_H_
#define __PWM_CHIPSIE_H_

#include "WPILib.h"

class PWMChip {
public:
	PWMChip(uint8_t module, uint8_t address);

	/**
	 * Channel is from 0 to 15
	 * Ontime is value from 0.0 to 1.0
	 */
	void setChannel(int channel, float ontime);
	void getChannel(int channel, bool &highfull, int &high, bool &lowfull, int &low);

	void getSleep(bool &asleep);
	void setSleep(bool asleep);
	
	void setTotemPole(bool on);
    
    void setAllChannels(float ontime);

	/** Takes a value in Hz. Wakes up the PWM chip
	 * Range is 23.8 Hz to 6104 Hz */
	void setPreScale(float update_rate);

	virtual ~PWMChip();
private:
	/** highstart and lowstart are the time per period in fractions
	 * of 4096 (range 0-4095) of the update period. If one is PWM_OFF (-1).
	 * Each cycle, at time lowstart, the channel drops low; at time highstart, the 
	 * channel goes high */
	void writeChannel(uint8_t channel, int highstart, int lowstart);
	void writeSubChannel(uint8_t subchannel, bool full, uint32_t period);
	void getSubChannel(uint8_t subchannel, bool &full, int &start);
	
	void setRegisterBit(uint8_t reg, uint8_t mask, bool high);
	void getRegisterBit(uint8_t reg, uint8_t mask, bool &value);
	
	I2C* pwm_bank;
	uint8_t address;
};

#endif // __PWM_CHIPSIE_H_
