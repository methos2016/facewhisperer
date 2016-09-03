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
    Serial.println("Ready to experiment!");

    reset_usb();
    reset_target();
    Usb.Init();

    // Wait until the device has been found and addressed
    do {
        led_error(1);
        Usb.Task();
        led_error(0);
    } while (Usb.getUsbTaskState() != USB_STATE_RUNNING);

    led_ok(1);

    // Set a small NAK limit for EP0, so we fail faster if/when the device NAKs
    Usb.getEpInfoEntry(1, 0)->bmNakPower = 4;

    // Try to do a ridiculous long descriptor read, and hexdump whatever we get back.
    memset(buffer, 0, sizeof buffer);
    trigger_high();
    int result = Usb.getConfDescr(1, 0, sizeof buffer, 0, buffer);
    trigger_low();

    // ctrlReq doesn't tell us how many IN packets we got back successfully,
    // so to keep the output concise we'll just dump everything save for trailing zeroes.

    int guessed_length = sizeof buffer;
    while (guessed_length && buffer[guessed_length - 1] == 0) {
        guessed_length--;
    }

    Serial.print("code ");
    Serial.println(result);
    Serial.print("len ");
    Serial.println(guessed_length);
    Serial.println("raw:");
    for (int i = 0; i < guessed_length; i++) {
        Serial.write(buffer[i]);
    }

    // Start over with a software reset
    reset_xmega();

    return 0;
}

