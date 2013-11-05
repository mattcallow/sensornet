#include <string.h>
#ifdef __AVR__
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/crc16.h>
#include <util/atomic.h>
#include <util/delay.h>
#endif
#include "sensornet.h"
#include "debug.h"

#ifdef USE_MILLIS
static volatile uint8_t f_ms=0;
static volatile unsigned long ms=0;
unsigned long millis(void)
{
	unsigned long ret;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		ret = ms;
	}
	return ret;
}
#endif

#ifdef USE_WDT
volatile uint8_t wdto=0;
volatile unsigned long secs=0;
unsigned long seconds(void)
{
	return secs;
}

ISR(WDT_vect)
{
	wdto=1;
}

void wd_sleep(void)
{
#ifdef SN_TX_CAPABLE
	// make sure that we have transmitted all data before sleeping
	// otherwise we may sleep mid transmit, leaving the transmitter enabled
	sn_wait_tx();
#endif
	wdto=0;
	MCUSR=0;
	WDTCR = _BV(WDIE) | _BV(WDP3) | _BV(WDP0);
	while (!wdto)
	{
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode();
		sleep_disable();
	}
	wdt_disable();
	secs+=8;
}

#endif

volatile uint8_t adc_done=false;

ISR(ADC_vect)
{
	adc_done = true;
}

uint8_t read_adc(uint8_t mux)
{
	uint16_t adc_val=0;
	ADMUX = mux | 0b00100000; // ensure left adjusted
#if ((F_CPU == 8000000) || (F_CPU == 4000000))
	ADCSRA =  0b10001110; // Enable ADC with clock/64, interrupt enable
#elif (F_CPU == 1000000)
	ADCSRA =  0b10001011; // Enable ADC with clock/8, interrupt enable
#else
#error Invalid F_CPU. Only 1MHz, 4Mhz, and 8Mhz supported
#endif
	_delay_ms(2); // settling time - only really required for reading the bandgap
	set_sleep_mode(SLEEP_MODE_ADC);
	// take 4 samples... 
	for (int i=0;i<4;i++) {
		adc_done = false;
		ADCSRA |= 0b01000000; // start the conversion
		cli();
		while (!adc_done) {
			sleep_enable();
			sei();
			sleep_cpu();
			sleep_disable();
		}
		sei();
		adc_val += ADCH;
	}
	ADCSRA &=  0b01111111; // turn off ADC 
	// then return the average
	adc_val >>=2;
	return (uint8_t)adc_val&0xff;
}

/*
 * vim: ts=4 sw=4
 */
