/*
 * sensornet_conf.h
 *
 *  Created on: 15/01/2013
 *      Author: matt
 */
#ifndef SENSORNET_CONF_H_
#define SENSORNET_CONF_H_

#include <avr/io.h>
#ifndef SN_ADDRESS_ME
#define SN_ADDRESS_ME 	1
#endif


#ifndef SN_MAX_HOPS
#define SN_MAX_HOPS 	3
#endif

#define SN_RX_PIN_NUMBER 3
#define SN_RX_DDR DDRB
#define SN_RX_PORT PORTB
#define SN_RX_PIN PINB
#define SN_RX_MASK _BV(SN_RX_PIN_NUMBER)


#define SN_TX_PIN_NUMBER 2
#define SN_TX_DDR DDRB
#define SN_TX_PORT PORTB
#define SN_TX_PIN PINB
#define SN_TX_MASK _BV(SN_TX_PIN_NUMBER)


#define SN_LED_PIN_NUMBER 0
#define SN_LED_DDR DDRB
#define SN_LED_PORT PORTB
#define SN_LED_PIN PINB
#define SN_LED_MASK _BV(SN_LED_PIN_NUMBER)

// Port B3/ADC3
#define SN_TEMP_PIN_NUMBER 3
#define SN_TEMP_DDR DDRB
#define SN_TEMP_PORT PORTB
#define SN_TEMP_PIN PINB
#define SN_TEMP_MASK _BV(SN_TEMP_PIN_NUMBER)
#define SN_TEMP_MUX 0b10000011	/* ADC3, internal ref */

#undef SN_RX_CAPABLE
#define SN_TX_CAPABLE
#define USE_MILLIS

typedef struct {
	uint8_t my_address;
	
} config_data_t;

// RF bit rate
#define SN_SPEED 2000

#endif /* SENSORNET_CONF_H_ */
