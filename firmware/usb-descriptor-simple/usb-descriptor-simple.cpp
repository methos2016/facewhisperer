#include "hal.h"
#include "Usb.h"
#include <stdint.h>
#include <stdlib.h>

USB  Usb;

void wait_for_go(void);
void wait_for_usb_device(void);
uint8_t streaming_ctrl_read(uint8_t addr, uint8_t ep, uint8_t bmReqType, uint8_t bRequest,
                            uint8_t wValLo, uint8_t wValHi, uint16_t wInd, uint16_t wLength);

int main()
{
    platform_init();
    Serial.begin(115200);

    wait_for_go();

    // Pull the target reset low; actual reset can take a somewhat unpredictable amount of time,
    // both due to the capacitance of the RST line charging, and the CPU starting up from an
    // internal 8 MHz oscillator
    reset_target();

    // Make sure reset has been asserted and I/Os have settled before sync'ing
    _delay_ms(10);

    // Resynchronize with the target using a GPIO signal it asserts after booting
    sync_to_posedge();

    // Initialize timers and USB only after sync'ing with the target, to avoid introducing jitter when we use the USB stack
    timer_init();
    reset_usb();
    Usb.Init();

    Serial.println();

    wait_for_usb_device();

    // Try to do a descriptor read that we can glitch into something else, and stream each received IN packet back to the host
    trigger_high();
    streaming_ctrl_read(1, 0, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, 0, USB_DESCRIPTOR_CONFIGURATION, 0x0000, 4096);
    trigger_low();

    // Start over with a software reset
    reset_xmega();

    return 0;
}

void wait_for_go()
{
    // Wait for a "go" signal from the host.
    Serial.println("Waiting");
    while (Serial.read() != '\n');
}

void wait_for_usb_device()
{
    // Wait until the device has been found and addressed
    do {
        led_error(1);
        Usb.Task();
        if (Usb.getUsbTaskState() == USB_STATE_ERROR ||
            Usb.getUsbTaskState() == USB_STATE_DETACHED) {
            reset_xmega();
        }
        led_error(0);
    } while (Usb.getUsbTaskState() != USB_STATE_RUNNING);
    led_ok(1);
}

uint8_t streaming_ctrl_read(uint8_t addr, uint8_t ep, uint8_t bmReqType, uint8_t bRequest,
                            uint8_t wValLo, uint8_t wValHi, uint16_t wInd, uint16_t wLength)
{
    // Send an arbitrary SETUP packet, then keep issuing and logging INs until failure.
    // Note that there's a high-level function Usb.ctrlReq() we could use except that
    // it needs enough RAM to buffer the result. This usage is odd enough to warrant
    // poking at the MAX3421E more directly.

    uint8_t rcode;
    SETUP_PKT setup_pkt;

    // Use a shortish NAK limit, to fail faster
    uint16_t nak_limit = 1 << 5;

    Usb.regWr(rPERADDR, addr);

    setup_pkt.ReqType_u.bmRequestType = bmReqType;
    setup_pkt.bRequest = bRequest;
    setup_pkt.wVal_u.wValueLo = wValLo;
    setup_pkt.wVal_u.wValueHi = wValHi;
    setup_pkt.wIndex = wInd;
    setup_pkt.wLength = wLength;

    Usb.bytesWr(rSUDFIFO, 8, (uint8_t*) &setup_pkt);
    rcode = Usb.dispatchPkt(tokSETUP, ep, nak_limit);
    if (rcode)
        return rcode;

    Serial.println("IN");

    // Expect DATA1 next
    Usb.regWr(rHCTL, bmRCVTOG1);

    uint32_t total_bytes = 0;

    // Keep going until we fail, never finish the ctrl request successfully.
    while (1) {

        rcode = Usb.dispatchPkt(tokIN, ep, nak_limit);
        if (rcode) {
            Serial.print("rcode ");
            Serial.print(rcode);
            Serial.print(" total ");
            Serial.print(total_bytes);
            Serial.println("\n\n");
            return rcode;
        }

        uint8_t byte_count = Usb.regRd(rRCVBC);
        total_bytes += byte_count;

        for (uint8_t i = 0; i < byte_count; i++) {
            uint8_t byte = Usb.regRd(rRCVFIFO);
            Serial.print(byte >> 4, HEX);
            Serial.print(byte & 0xF, HEX);
        }
        Serial.println();

        // Free this packet's buffer
        Usb.regWr(rHIRQ, bmRCVDAVIRQ);
    }
}

