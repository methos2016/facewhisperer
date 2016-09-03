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
	Usb.Init();
	Serial.println("Ready to experiment!");

	while (1) {
		led_ok(0);
		reset_target();

		// Wait until the device has been found and addressed
		do {
			Usb.Task();
		} while (Usb.getUsbTaskState() != USB_STATE_RUNNING);

		led_ok(1);

		// Try to do a ridiculous long descriptor read, and hexdump whatever we get back.
		memset(buffer, 0, sizeof buffer);
		int result = Usb.getConfDescr(1, 0, sizeof buffer, 0, buffer);

		Serial.print("code ");
		Serial.println(result);
		Serial.println("raw:");
		for (int i = 0; i < sizeof buffer; i++) {
			Serial.write(buffer[i]);
		}
	}

	return 0;
}

