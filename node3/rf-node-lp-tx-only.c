/*
 * rf-node-lp-tx-only.c
 * A low power (battery) transmit only node
 * Uses WDT for periodic wakeup
 * 
 */
#ifdef __PIC18__
#include <xc.h>
#endif
#ifdef __AVR__
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/fuse.h>
FUSES = 
{
	.low = LFUSE_DEFAULT,
	.high = HFUSE_DEFAULT,
	.extended = EFUSE_DEFAULT,
};
#endif
#include <stdio.h>
#include "sensornet.h"
#include "config.h"
#include "macros.h"
#include "debug.h"
#include "onewire.h"
#include "task.h"

config_data_t config = { 
	.my_address=SN_ADDRESS_ME, 
	.vsupply_cal=288836,  // specific per device
};
sn_message_t msg; // buffer used for transmit and receive

#ifdef SN_TX_CAPABLE
static volatile uint8_t seqnum=0;
static volatile uint8_t sensor_no=0;
static void send_status(uint8_t dest);
static void send_sensor_data(uint8_t dest);
#endif

static volatile uint8_t mcusr_save;
TASK_LIST_START
TASK(STATUS_PERIOD, STATUS_PERIOD, send_status, SN_ADDRESS_BROADCAST),
TASK(0, SENSOR_PERIOD, send_sensor_data, SN_ADDRESS_BROADCAST)
TASK_LIST_END

int main(void)
{
#ifdef __AVR__
	mcusr_save = MCUSR;
	MCUSR = 0;
	wdt_disable();
	CLKPR=0x80; // enable clock prescale change
	CLKPR=0; // change prescale to 1
	DBG_STRP_U8H_NL("Start. num_tasks=", num_tasks);
#ifdef TEMP_PORT
	// configure pin for analog
	DDR(TEMP_PORT) &= ~TEMP_MASK; // set as input
	PORT(TEMP_PORT) &= ~TEMP_MASK;	// pull-ups off
	DIDR0 |= TEMP_MASK; // disable digital input buffer
#endif
#ifdef ISR_DBG_PORT
	DDR(ISR_DBG_PORT) |= ISR_DBG_MASK; // set as output
	PORT(ISR_DBG_PORT) &= ~ISR_DBG_MASK; // turn off
#endif
#ifdef LED_PORT
	DDR(LED_PORT) |= LED_MASK; // set as output
	PORT(LED_PORT) &= ~LED_MASK; // turn off
#endif
#ifdef OW_PORT
	ow_init();
#endif
// TODO - configure for PIC
#endif
	sn_setup();
	// send 2 status messages, in case one is lost
	send_status(SN_ADDRESS_BROADCAST);
	sn_delay_ms(500);
	send_status(SN_ADDRESS_BROADCAST);
	sn_delay_ms(500);
/*
	for (;;) {
		send_sensor_data(SN_ADDRESS_BROADCAST);
		sn_delay_ms(500);
	}
*/
	task_loop();
}

/*
 * return temperature in degrees C, using external TMP36 sensor
 * Sensor outputs 750mV at 25C, 500mV at 0C
 * scale factor is 10mV/C
 */
/*
static int16_t read_temperature_tmp36(void)
{
	return (1100*(uint32_t)read_adc(TEMP_MUX))/256 - 500;
}
*/

/*
 * return temperature in degrees C, using internal AVR sensor
 */
/*
static uint16_t read_temperature_int(void)
{
	return read_adc(INT_TEMP_MUX);
	//return read_adc(0b10001111) - 234; // 234 is 'calibrated' temperature offset
}
*/

/*
 * Read temperature from a DS18B20
 * Returns value as 1/10 of degree C
 * Only works for DS18B20, and only if it's the only device on 1-wire bus
 */
static int16_t read_temperature_ow(void)
{
#ifdef OW_PORT
	uint8_t data[9];
	int16_t raw;
	float celsius;
	ow_reset();
	ow_skip();
	ow_write(0x44, 1); // issue convert command. Apply power
	sn_delay_ms(750); // wait for conversion
	ow_reset();
	ow_skip();
	ow_write(0xbe, 0); // read scratchpad
	for (int i=0;i<9;i++) {
		data[i] = ow_read();
	}
	uint8_t crc = ow_crc8(data, 8);
	if (crc != data[8]) return 0;
	raw = (data[1] << 8) | data[0];
	// valid for DS18B20 only...
	uint8_t cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
	celsius = (float)raw/16.0;
	return (int16_t)(celsius*10);
#else
	return 0;
#endif
}

static uint16_t read_light_level(void)
{
	// TODO
	return 0;
}
static uint16_t read_supply_raw(void)
{
	return read_adc(0b00101100); // select Vcc as ref, read internal bandgap
}
/* 
 * Return supply voltage as 16 bit int (in mv)
 */
static uint16_t read_supply_v(void)
{
	uint32_t ret;
	ret = config.vsupply_cal/read_supply_raw();
	return ((uint16_t)ret);
}

/*
 * TASK send_sensor_data
 */
static void send_sensor_data(uint8_t dest)
{
	DBG_STRP("----\r\n");
	msg.header.source = config.my_address;
	msg.header.destination = dest;
	msg.header.max_hops = SN_MAX_HOPS;
	msg.header.type = SN_MESSAGE_SENSOR_DATA;
	sn_payload_sensor_data_t *p = (sn_payload_sensor_data_t *)msg.payload;

	switch (sensor_no++) {
	case 0:
		p->sensor = 0x22;
		p->data_valid = 1;
		p->data.analog16 = read_temperature_ow();
		msg.header.seq = seqnum++;
		DBG_STRP_U16H_NL("send_sensor_data 0x22=0x", p->data.analog16);
		break;
	case 1:
		p->sensor = 0x21;
		p->data_valid = 1;
		p->data.analog16 = read_supply_v();
		msg.header.seq = seqnum++;
		DBG_STRP_U8H_NL("send_sensor_data 0x21=0x", p->data.analog16);
		break;
	case 2:
		p->sensor = 0x23;
		p->data_valid = 0; // TODO
		p->data.analog16 = read_light_level();
		msg.header.seq = seqnum++;
		DBG_STRP_U8H_NL("send_sensor_data 0x23=0x", p->data.analog16);
		break;
/*
	case 3:
		p->sensor = 0x82;
		p->data_valid = 1;
		p->data.counter32 = (uint32_t)seconds();
		msg.header.seq = seqnum++;
		//DBG_STR_U8H_NL("send_sensor_data 0x82=0x", p->data.counter32);
		break;
	case 4:
		p->sensor = 0x20;
		p->data_valid = 1;
		p->data.analog16 = read_supply_raw();
		msg.header.seq = seqnum++;
		DBG_STRP_U8H_NL("send_sensor_data 0x20=0x", p->data.analog16);
		break;
*/
	default:
		sensor_no=0;
	}
	if (sensor_no != 0) {
		sn_send_wait((uint8_t *)&msg, SN_HEADER_LEN + sizeof(sn_payload_sensor_data_t));
		sn_delay_ms(100);
	}
}

/*
 * TASK send_status
 */
static void send_status(uint8_t dest)
{
	msg.header.source = SN_ADDRESS_ME;
	msg.header.destination = dest;
	msg.header.seq = seqnum++;
	msg.header.max_hops = SN_MAX_HOPS;
	msg.header.type = SN_MESSAGE_STATUS;
	sn_payload_status_t *p = (sn_payload_status_t *)msg.payload;
	p->status = mcusr_save;

	p->capabilities = CAPABILITY_BATTERY | CAPABILITY_TX ;
	p->max_routes = 0;
	p->supply_voltage = read_supply_v();
	sn_send_wait((uint8_t *)&msg, SN_HEADER_LEN + sizeof(sn_payload_status_t));
}

// vim: ts=4 sw=4
