#include "WPILib.h"
#include "PWMChip.h"

const uint8_t ADDRESS = 0x80; // 1 000000 x

bool testSleep(PWMChip* pwm) {
    bool asleep;
    pwm->getSleep(asleep);
    return asleep;
}

double linJoy(double i) {
	return (i+1.0)/2.0;
}

class I_TOO_SEE_YOU: public SimpleRobot {
	PWMChip* pwms;
	Joystick* joy;
public:
	void RobotInit() {
		printf("ROBOT INIT!\n");
		pwms = new PWMChip(1, ADDRESS);
		pwms->setTotemPole(true);
		pwms->setPreScale(100.0); // 100 Hz
        printf("Before: Is asleep: %s\n", testSleep(pwms) ? "YES" : "NO");
		pwms->setSleep(false);
        printf("After:  Is asleep: %s\n", testSleep(pwms) ? "YES" : "NO");
        
        joy = new Joystick(1);;
	}

	void Autonomous(void) {
        printf("AUTON\n");

        pwms->printChannel(0);
        pwms->setChannel(0, 0.75);
        pwms->printChannel(0);
	}

	void JoyToChannel() {
		pwms->setChannel(0, linJoy(joy->GetX()));
		pwms->setChannel(1, linJoy(joy->GetY()));
		pwms->setChannel(2, linJoy(joy->GetZ()));
		printf("-----\n");
        pwms->printChannel(0);
        pwms->printChannel(1);
        pwms->printChannel(2);
	}
	
	void OperatorControl(void) {
		printf("OPCONT\n");
		// we change LED1 frequency
		
		while (IsOperatorControl() && IsEnabled()) {
			JoyToChannel();
            Wait(0.05);
		}
	}

	void Test() {
		printf("TEST\n");
	}

	void Disabled() {
		printf("DISABLED\n");
		while (IsDisabled()) {
			JoyToChannel();
			Wait(0.05);
		}
	}
};

START_ROBOT_CLASS(I_TOO_SEE_YOU)
;

