#include "hal.h"
#include "Usb.h"
#include <stdint.h>
#include <stdlib.h>

USB  Usb;


int main()
{
	platform_init();
	init_uart();
	trigger_setup();

	Serial.print("Does this work?");

	Usb.Task();

	while (1) {
		trigger_high();
		trigger_low();
	}

	return 0;
}

