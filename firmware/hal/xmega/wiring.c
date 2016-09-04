/*
  wiring.c - Partial implementation of the Wiring API for the ATmega8.
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.c 808 2009-12-18 17:44:08Z dmellis $
*/

#include <stddef.h>
#include <avr/pgmspace.h>
#include "wiring_private.h"

// Some versions of avr-gcc on linux defines XXX_bp instead of _gp.
#if !defined ADC_SWEEP_gp
    #define ADC_SWEEP_gp  ADC_SWEEP0_bp
#endif
#if !defined ADC_EVSEL_gp
    #define ADC_EVSEL_gp  ADC_EVSEL0_bp
#endif
#if !defined ADC_EVACT_gp
    #define ADC_EVACT_gp  ADC_EVACT0_bp
#endif
#if !defined ADC_CH_GAINFAC_gp
    #define ADC_CH_GAINFAC_gp  ADC_CH_GAIN_1X_gc
#endif

// Not worried about long runtimes in this environment
volatile uint32_t millis_count = 0;

ISR(TCC0_OVF_vect)
{
  ++millis_count;
}

unsigned long millis(void)
{
	uint8_t oldSREG = SREG; // Save and restore the interrupt enable bit
	cli();
	unsigned long result = millis_count;
	SREG = oldSREG;

	return result;
}

uint64_t micros_huge(void)
{
	uint8_t oldSREG = SREG; // Save and restore the interrupt enable bit
	cli();
	uint64_t result = (millis_count * 1000ULL) + TCC0.CNT / (F_CPU / 1000000);
	SREG = oldSREG;

	return result;
}

unsigned long micros(void)
{
  uint8_t oldSREG = SREG; // Save and restore the interrupt enable bit
  cli();
  unsigned long result = (millis_count * 1000UL) + TCC0.CNT / (F_CPU / 1000000);
  SREG = oldSREG;

	return result;
}

void delayMicroseconds(unsigned int us)
{
  unsigned long start = micros();
  while (micros() - start <= us);
}

void delay(unsigned long ms)
{
	unsigned long start = millis();
	while (millis() - start <= ms);
}

void timer_init()
{
  millis_count = 0;

  /*************************************/
  /* Init real time clock for millis() */

  // Reset the clock
  TCC0.CTRLA    = TC_CLKSEL_OFF_gc;
  TCC0.CNT      = 0;
  TCC0.CTRLFCLR = TC0_DIR_bm | TC0_LUPD_bm;
  TCC0.CTRLFSET = TC_CMD_RESET_gc;

  /* Timer 1 uses the system clock as its source, overflows every microsecond.
   * Avoiding the prescaler for now, to avoid introducing another potential jitter source.
   */
  TCC0.PER      = F_CPU / 1000;
  TCC0.CTRLB    = TC_WGMODE_NORMAL_gc;
  TCC0.INTCTRLA = TC_OVFINTLVL_HI_gc;
  TCC0.CTRLA    = TC_CLKSEL_DIV1_gc;
}

void adc_init( ADC_t* adc ) {
        /* ADC INIT */

        adc->CTRLB   = 0 << ADC_CONMODE_bp      // unsigned conversion
                     | 0 << ADC_FREERUN_bp      // no freerun
                     | ADC_RESOLUTION_12BIT_gc  // 12bit resolution
                     ;

        // TODO: What should we use as analog ref?
        adc->REFCTRL = ADC_REFSEL_VCC_gc   // VCC/1.6 analog ref
                     | 0 << ADC_BANDGAP_bp // bandgap not enabled
                     | 0 << ADC_TEMPREF_bp // temerature reference not enabled
                     ;

        adc->EVCTRL = 0 << ADC_SWEEP_gp // Have to set it to something, so sweep only channel 0.
                    | 0 << ADC_EVSEL_gp // Have to set it to something, so event channels 0123.
                    | 0 << ADC_EVACT_gp // No event action
                    ;

        // TODO: What should we use as prescalar?
        // 128K times per second with 32Mhz sys clock. That's what the mega based
        // arduinos use. No idea if that is appropriate for xmegas.
        adc->PRESCALER = ADC_PRESCALER_DIV256_gc;

        adc->INTFLAGS = 0; // No interrupt on conversion complete

        /* CHANNEL INIT */

        // TODO: Perhaps we should create API so we can use all ADC channels, events, free run, etc.
        // TODO: Perhaps we should use ADC channel 3 rather than 0 ...

        adc->CH0.CTRL = 0 << ADC_CH_START_bp            // Don't start conversion yet
                      | 0 << ADC_CH_GAINFAC_gp          // 1x gain (2^0 is 1)
                      | ADC_CH_INPUTMODE_SINGLEENDED_gc // single ended
                      ;

        adc->CH0.INTCTRL = ADC_CH_INTMODE_COMPLETE_gc // Not really, below value turns interrupts off ...
                         | ADC_CH_INTLVL_OFF_gc       // Interrupt off
                         ;

        adc->CH0.INTFLAGS = 1; // Strangely enough, clears IF

        // Do CTRLA last so everything is initialized before we enable.
        adc->CTRLA   = 0 << ADC_CH0START_bp // don't start ADC
                     | 0 << ADC_FLUSH_bp    // don't flush
                     | 1 << ADC_ENABLE_bp   // enable
                     ;
}
