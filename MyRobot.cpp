#include "WPILib.h"
#include "PWMChip.h"

const uint8_t ADDRESS = 0x80; // 1 000000 x

bool testSleep(PWMChip* pwm) {
    bool asleep;
    pwm->getSleep(asleep);
    return asleep;
}

void printChannel(PWMChip* pwm, int channel) {
    bool hf, lf;
    int hs, ls;
    pwm->getChannel(channel, hf, hs, lf, ls);
    printf("%2d::HF %d| HS %d| LF %d|LS %d\n", channel, hf, hs, lf, ls);
}

class I_TOO_SEE_YOU: public SimpleRobot {
	PWMChip* pwms;
public:
	void RobotInit() {
		printf("ROBOT INIT!\n");
		pwms = new PWMChip(1, ADDRESS);
		pwms->setTotemPole(true);
		pwms->setPreScale(100.0); // 100 Hz
        printf("Before: Is asleep: %s\n", testSleep(pwms) ? "YES" : "NO");
		pwms->setSleep(false);
        printf("After:  Is asleep: %s\n", testSleep(pwms) ? "YES" : "NO");
	}

	void Autonomous(void) {
        printf("AUTON\n");

        printChannel(pwms, 0);
        printChannel(pwms, 1);
        
        pwms->setAllChannels(0.673);

        printChannel(pwms, 0);
        printChannel(pwms, 1);

        pwms->setChannel(0, 0.200);
        
        printChannel(pwms, 0);
        printChannel(pwms, 1);
	}

	void OperatorControl(void) {
        const int CYCLES = 200;
		printf("OPCONT\n");
		// we change LED1 frequency
		bool hf, lf;
		int hs, ls;
		
		int i = 0;
		while (IsOperatorControl() && IsEnabled()) {
            float time = i / CYCLES;
			printf("Write: %1.4f\n", time);
            pwms->setChannel(0, time);

            printChannel(pwms, 0);

			i = (i+1)%(CYCLES+1);

            Wait(0.05);
		}
	}

	void Test() {
		printf("TEST\n");
	}

	void Disabled() {
		printf("DISABLED\n");
	}
};

START_ROBOT_CLASS(I_TOO_SEE_YOU)
;

