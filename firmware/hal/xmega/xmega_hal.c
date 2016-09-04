/*
    This file is part of the ChipWhisperer Example Targets
    Copyright (C) 2012-2015 NewAE Technology Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <avr/sleep.h>
#include "hal.h"
#include "xmega_hal.h"

void platform_init(void)
{
    OSC.XOSCCTRL = 0x00;
    OSC.PLLCTRL = 0x00;
    OSC.CTRL |= OSC_XOSCEN_bm;

    // Wait for external clock
    while((OSC.STATUS & OSC_XOSCRDY_bm) == 0);

    // Switch clock source
    CCP = CCP_IOREG_gc;
    CLK.CTRL = CLK_SCLKSEL_XOSC_gc;

    // Turn off other sources besides external
    OSC.CTRL = OSC_XOSCEN_bm;

 #if PLATFORM == CW303
    PORTA.DIRSET = PIN5_bm | PIN6_bm;
    PORTA.OUTSET = PIN5_bm | PIN6_bm;
 #endif

 #if PLATFORM == FACEWHISPERER
    PORTA.DIRSET = PIN0_bm | PIN5_bm | PIN6_bm | PIN7_bm;
    PORTA.OUTSET = PIN5_bm | PIN6_bm;
 #endif

    // Enable interrupts.
    PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
    sei();
}

#if PLATFORM == FACEWHISPERER
void reset_target()
{
    // Pulse the target reset transistor
    PORTA.DIRSET = PIN7_bm;
    PORTA.OUTSET = PIN7_bm;
    _delay_us(1000);
    PORTA.OUTCLR = PIN7_bm;
}

void reset_usb()
{
    // Max3241e hardware reset
    PORTC.DIRSET = PIN1_bm;
    PORTC.OUTCLR = PIN1_bm;
    _delay_us(20);

    // Leave RST with a pull-up
    PORTC.PIN1CTRL |= PORT_OPC_PULLUP_gc;
    PORTC.DIRCLR = PIN1_bm;
}
#endif


void reset_xmega()
{
    cli();
    CCP = 0xD8;
    RST.CTRL = RST_SWRST_bm;
}


static volatile bool aca_ac0_flag;

ISR(ACA_AC0_vect)
{
    aca_ac0_flag = true;
}

void xmega_comparator_sync(AC_MUXPOS_t pin, AC_INTMODE_t edge, int millivolts)
{
    // Use the Analog Comparator unit to wait for an edge
    // with low jitter and a programmable threshold.

    // Input is in millivolts for convenience; convert
    // to the nearest level in the Vcc/64 divider network.
    int scalefac = (millivolts * 10000000UL + 257812UL) / 515625UL;
    if (scalefac < 1) scalefac = 1;
    if (scalefac > 64) scalefac = 64;
    ACA.CTRLB = scalefac - 1;

    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();

    ACA.CTRLA = 0;
    ACA.WINCTRL = 0;
    ACA.AC0MUXCTRL = pin | AC_MUXNEG_SCALER_gc;
    ACA.AC0CTRL = edge | AC_INTLVL_HI_gc | AC_HYSMODE_SMALL_gc | AC_ENABLE_bm;
    aca_ac0_flag = false;

    // Go into idle sleep (with peripherals all enabled) to resume with predictable latency
    do {
        sleep_cpu();
    } while (!aca_ac0_flag);

    ACA.AC0CTRL = 0;
    sleep_disable();
}


#if HWCRYPTO
#include "XMEGA_AES_driver.h"
static uint8_t enckey[16];

void HW_AES128_Init(void)
{
    return;
}

void HW_AES128_LoadKey(uint8_t * key)
{
	for(uint8_t i=0; i < 16; i++){
		enckey[i] = key[i];
	}
}

void HW_AES128_Enc(uint8_t * pt)
{
    AES_encrypt(pt, pt, enckey);
}
#endif