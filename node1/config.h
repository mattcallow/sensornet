/*
 * config.h
 *
 *  Created on: 15/01/2013
 *      Author: matt
 */
#ifndef _CONFIG_H_
#define _CONFIG_H_

#if defined(__AVR__)
#include <avr/io.h>
#elif defined(__PIC18__)
#include <xc.h>
#endif
#include <stdint.h>

#define MW_RADIO
#undef VW_RADIO

#ifndef SN_ADDRESS_ME
#define SN_ADDRESS_ME 	1
#endif


#ifndef SN_MAX_HOPS
#define SN_MAX_HOPS 	3
#endif

#define TX_PORT	B
#define TX_BIT 2
#define TX_MASK _BV(TX_BIT)


#define LED_PORT B
#define LED_BIT 0
#define LED_MASK _BV(LED_BIT)

// Port B3/ADC3
//#define TEMP_PORT B
#undef TEMP_PORT
#define TEMP_BIT 3
#define TEMP_MASK _BV(TEMP_BIT)
#define TEMP_MUX 0b10100011	/* ADC3, internal ref */
//#define SN_TEMP_MUX 0b10100010	/* ADC2, internal ref */

#define OW_PORT B
#define OW_BIT 1
#define OW_MASK _BV(OW_BIT)

#define INT_TEMP_MUX 0b10101111	/* ADC4, internal ref */


#undef SERIAL_DEBUG

#ifdef SERIAL_DEBUG
#define DBG_PORT	B
#define DBG_BIT 	0
#endif

#undef SN_RX_CAPABLE
#define SN_TX_CAPABLE
#undef USE_MILLIS
#define USE_WDT

typedef struct {
	uint8_t my_address;
	uint32_t vsupply_cal;
} config_data_t;

// RF bit rate
#define SN_SPEED 2000

#define STATUS_PERIOD  3600 // seconds
#define SENSOR_PERIOD    60 // seconds

#endif /* _CONFIG_H_ */
