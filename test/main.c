/*
 * main.c
 * test file
 * 
 */
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/delay.h>
#include "debug.h"
#include "sensornet.h"


int main(void)
{
	dbg_init();
	dbg_msg("\r\nStart");
	for (;;) {
		dbg_str_u8h_nl("ADCH=",read_adc(ADC_REF_1_1 | ADC_SE_ADC3));
		//uint16_t mv = 10*(110 * read_adc(ADC_REF_1_1 | ADC_SE_ADC3)/256);
		//dbg_str_u16h_nl("MV=",mv);
		_delay_ms(500);

	}
}


// vim: ts=4 sw=4
