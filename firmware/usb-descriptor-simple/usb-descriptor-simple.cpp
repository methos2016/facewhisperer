#include "hal.h"
#include "Usb.h"
#include <stdint.h>
#include <stdlib.h>

USB  Usb;


int main()
{
	platform_init();
	trigger_setup();

    Serial.begin(38400);
	Serial.print("Hello World\n");

	pinMode(18, OUTPUT);
	pinMode(19, OUTPUT);
	digitalWrite(18, HIGH);
	digitalWrite(19, HIGH);
	digitalWrite(18, LOW);
	digitalWrite(19, LOW);

	Usb.Task();

	Serial.print("Made it through USB\n");

	led_ok(1);

	while (1) {
		
		for (int i = 0; i < 100; i++) {
			trigger_high();
			trigger_low();
		}

		reset_gate(1);
		delay(1);
		reset_gate(0);

		delay(100);

	}

	return 0;
}

