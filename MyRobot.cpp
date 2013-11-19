#include "WPILib.h"
#include "PWMChip.h"

const uint8_t ADDRESS = 0x00; // TODO set this

class I_TOO_SEE_YOU: public SimpleRobot {
	PWMChip* pwms;
public:
	I_TOO_SEE_YOU(void) {
		pwms = new PWMChip(1, ADDRESS);
		pwms->setTotemPole(true);
		pwms->setPreScale(100.0); // 100 Hz
	}

	void Autonomous(void) {
		printf("AUTON\n");
	}

	void OperatorControl(void) {
		printf("OPCONT\n");
		while (IsOperatorControl() && IsEnabled()) {
			Wait(0.1);
		}
	}

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
				pwms->writeChannel(0, PWMChip::PWM_FULL);
				break;
			case 1:
				pwms->writeChannel(0, 0);
				break;
			case 2:
				pwms->writeChannel(0, 2047);
				break;
			case 3:
				pwms->writeChannel(0, 1023);
				break;
			}

			Wait(0.005);
		}
	}
};

START_ROBOT_CLASS(I_TOO_SEE_YOU)
;

