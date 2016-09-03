#include "hal.h"
#include "Usb.h"
#include <stdint.h>
#include <stdlib.h>

USB  Usb;
uint8_t buffer[4096];



int main()
{
	platform_init();
    Serial.begin(38400);

	Serial.println("Ready!");

	SPI.begin();
	while (1) {
		reset_target();

		led_error(1);
		SPI.transfer(0x55);
		led_error(0);

		led_ok(1);
		delay(100);
		led_ok(0);
	}

	Usb.Init();
	Serial.println(Usb.getVbusState());

	while (1) {
		Usb.Task();
//		Serial.println(Usb.getUsbTaskState());
	}

	// if (result < 0) {
	// 	led_error(1);
	// } else {
	// 	led_ok(1);
	// }

	// while (1) {
	// 	Usb.Task();
	// }

	return 0;
}

