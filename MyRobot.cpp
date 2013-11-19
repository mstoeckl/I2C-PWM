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
		// we change LED1 frequency
		int i = 0;
		while (IsOperatorControl() && IsEnabled()) {
			switch (i) {
			case 0:
				pwms->setChannel(0, 1.0);
				break;
			case 1:
				pwms->setChannel(0, 0.0);
				break;
			case 2:
				pwms->setChannel(0, 0.5);
				break;
			case 3:
				pwms->setChannel(0, 0.25);
				break;
			}

			Wait(1.0);

			i++;
		}
	}

	void Test() {
		printf("TEST\n");
	}

	void Disabled() {
		printf("Disabled\n");
	}
};

START_ROBOT_CLASS(I_TOO_SEE_YOU)
;

