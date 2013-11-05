/*
 * rf-node.c
 * Main file for a RF sensor node, using sensornet
 * 
 */
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "sensornet.h"

sn_message_t msg; // buffer used for transmit and receive

#ifdef SN_RX_CAPABLE
static void process_msg(void);
#endif
#ifdef SN_TX_CAPABLE
volatile uint8_t seqnum __attribute__ ((section (".noinit")));
static void send_status(uint8_t dest);
static void send_sensor_data(uint8_t dest);
#endif
#define STATUS_PERIOD  3600000UL // ms
#define SENSOR_PERIOD    1000UL // ms

#ifdef USE_MILLIS
static unsigned long last_status=0;
static unsigned long last_sensor_data=0;
#endif
volatile unsigned long delay_counter;
static volatile uint8_t mcusr_save;

int main(void)
{
	mcusr_save = MCUSR;
	MCUSR = 0;
	wdt_disable();
	if (!(mcusr_save & WDRF))
	{
		seqnum = 0;
	}
	
#ifdef SN_TEMP_DDR
	// configure pin for analog
	SN_TEMP_DDR &= ~SN_TEMP_MASK; // set as input
	SN_TEMP_PORT &= ~SN_TEMP_MASK;	// pull-ups off
	DIDR0 |= SN_TEMP_MASK; // disable digital input buffer
#endif
	SN_LED_DDR |= SN_LED_MASK; // set as output
	SN_LED_PORT &= ~SN_LED_MASK;
	vw_setup();
#ifdef SN_RX_CAPABLE
	vw_rx_start();
#endif
	for (int i=0;i<3;i++)
	{
		send_status(SN_ADDRESS_BROADCAST);
		vw_wait_tx();
	}
	for (;;) {
#ifdef SN_RX_CAPABLE
		if (vw_have_message()) {
			uint8_t len = sizeof(msg);
			if (vw_get_message((uint8_t *)&msg, &len)) {
				if (msg.header.destination == SN_ADDRESS_ME|| msg.header.destination == SN_ADDRESS_BROADCAST) {
					process_msg();
				} else {
					// TODO - consider routing here
				}
			} else {
				// bad message
			}
		}
#endif
#ifdef SN_TX_CAPABLE
		// Process any outgoing stuff
		if (millis() - last_status > STATUS_PERIOD) {
			send_status(SN_ADDRESS_BROADCAST);
		}
		if (millis() - last_sensor_data > SENSOR_PERIOD) {
			send_sensor_data(SN_ADDRESS_BROADCAST);
		}
#endif // SN_TX_CAPABLE
#ifdef USE_MILLIS
		//set_sleep_mode(SLEEP_MODE_IDLE);
		//sleep_enable();
		//sleep_cpu();
		//sleep_disable();
#endif
	}
}

static uint16_t read_adc(uint8_t mux)
{
	uint16_t adc_val;
	// read battery level
	ADMUX = mux & 0b11011111; // ensure right adjusted
#if (F_CPU == 8000000)
	ADCSRA =  0b10001110; // Enable ADC with clock/64, interrupt enable
#elif (F_CPU == 1000000)
	ADCSRA =  0b10001011; // Enable ADC with clock/8, interrupt enable
#else
#error Need 8Mhz or 1Mhz clock
#endif
	adc_val = 0;
	set_sleep_mode(SLEEP_MODE_ADC);
	// take 8 samples... 
	for (int i=0;i<8;i++) {
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
		adc_val += ADCL;
		adc_val += (ADCH << 8);
	}
	ADCSRA &=  0b01111111; // turn off ADC 
	// then return the average
	return (adc_val >> 3);
}

/*
 * return temperature in degrees C, using external TMP36 sensor
 * Sensor outputs 750mV at 25C, 500mV at 0C
 * scale factor is 10mV/C
 */
static int16_t read_temperature_tmp36(void)
{

	return read_adc(SN_TEMP_MUX);
	//return (1100*(uint32_t)read_adc(SN_TEMP_MUX))/1024 - 500;
}
/*
 * return temperature in degrees C, using internal AVR sensor
 */
static uint16_t read_temperature_int(void)
{
	return read_adc(0b10001111) - 234; // 234 is 'calibrated' temperature offset
}

static uint16_t read_light_level(void)
{
	// TODO
	return 0;
}
/* 
 * Return supply voltage as 16 bit int (in mv)
 */
static uint16_t read_supply_v(void)
{
	return 275000/(read_adc(0b00101100) >> 2); // select Vcc as ref, read internal bandgap
}
#ifdef SN_TX_CAPABLE
static void send_sensor_data(uint8_t dest)
{
#ifdef USE_MILLIS
	last_sensor_data = millis();
#endif
	msg.header.source = SN_ADDRESS_ME;
	msg.header.destination = dest;
	msg.header.max_hops = SN_MAX_HOPS;
	msg.header.type = SN_MESSAGE_SENSOR_DATA;
	sn_payload_sensor_data_t *p = (sn_payload_sensor_data_t *)msg.payload;
#ifdef USE_MILLIS
	p->sensor = 0x80;
	p->data_valid = 1;
	p->data.counter32 = (uint32_t)millis();
	msg.header.seq = seqnum++;
	vw_send((uint8_t *)&msg, SN_HEADER_LEN + sizeof(sn_payload_sensor_data_t));
#endif
	p->sensor = 0x21;
	p->data_valid = 1;
	p->data.analog16 = read_supply_v();
	msg.header.seq = seqnum++;
	vw_send((uint8_t *)&msg, SN_HEADER_LEN + sizeof(sn_payload_sensor_data_t));
	p->sensor = 0x22;
	p->data_valid = 1;
	p->data.analog16 = read_temperature_tmp36();
	msg.header.seq = seqnum++;
	vw_send((uint8_t *)&msg, SN_HEADER_LEN + sizeof(sn_payload_sensor_data_t));
	p->sensor = 0x30;
	p->data_valid = 1;
	p->data.analog16 = read_temperature_int();
	msg.header.seq = seqnum++;
	vw_send((uint8_t *)&msg, SN_HEADER_LEN + sizeof(sn_payload_sensor_data_t));
	p->sensor = 0x23;
	p->data_valid = 0; // TODO
	p->data.analog16 = read_light_level();
	msg.header.seq = seqnum++;
	vw_send((uint8_t *)&msg, SN_HEADER_LEN + sizeof(sn_payload_sensor_data_t));
}

static void send_status(uint8_t dest)
{
#ifdef USE_MILLIS
	last_status = millis();
#endif
	msg.header.source = SN_ADDRESS_ME;
	msg.header.destination = dest;
	msg.header.seq = seqnum++;
	msg.header.max_hops = SN_MAX_HOPS;
	msg.header.type = SN_MESSAGE_STATUS;
	sn_payload_status_t *p = (sn_payload_status_t *)msg.payload;
	p->status = mcusr_save;

	p->capabilities = CAPABILITY_MAINS
#ifdef SN_TX_CAPABLE
			| CAPABILITY_TX
#endif
#ifdef SN_RX_CAPABLE
			| CAPABILITY_RX
#endif
			;
	p->max_routes = 0;
	p->supply_voltage = read_supply_v();
	vw_send((uint8_t *)&msg, SN_HEADER_LEN + sizeof(sn_payload_status_t));
}
#endif

#ifdef SN_RX_CAPABLE
static void process_msg(void)
{
	switch (msg.header.type) {
	case SN_MESSAGE_AYT:
		send_status(msg.header.source);
		break;
	default:
		break;
	}
}
#endif


// vim: ts=4 sw=4
