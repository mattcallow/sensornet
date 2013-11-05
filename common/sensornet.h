/*
 * sensornet.h
 *
 *  Created on: 15/01/2013
 *      Author: matt
 *
 * Based on VirtualWrite from    Mike McCauley (mikem@open.com.au)
 * Copyright (C) 2008 Mike McCauley
 */

#ifndef SENSORNET_H_
#define SENSORNET_H_

#include <string.h>
#include "config.h"

#ifdef __AVR_ATtiny85__
#define ADC_SE_ADC0 	0b00000000
#define ADC_SE_ADC1 	0b00000001
#define ADC_SE_ADC2 	0b00000010
#define ADC_SE_ADC3 	0b00000011
#define ADC_SE_VBG  	0b00001100
#define ADC_SE_GND  	0b00001101
#define ADC_SE_ADC4 	0b00001111
#define ADC_REF_VCC	0b00000000
#define ADC_REF_1_1	0b10000000
#endif

// Maximum number of bytes in a message, counting the byte count and FCS
#define SN_MAX_MESSAGE_LEN 30

// The maximum payload length
#define SN_MAX_PAYLOAD SN_MAX_MESSAGE_LEN-3

#include "sn_messages.h"

#define SN_ADDRESS_MASTER 		0x00
#define SN_ADDRESS_BROADCAST	0x3f

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef __AVR__
#include <avr/io.h>
#endif




#define SN_RX_SAMPLES_PER_BIT 8
#ifdef SN_RX_CAPABLE
// The size of the receiver ramp. Ramp wraps modulu this number
#define SN_RX_RAMP_LEN 160


// Ramp adjustment parameters
// Standard is if a transition occurs before SN_RAMP_TRANSITION (80) in the ramp,
// the ramp is retarded by adding SN_RAMP_INC_RETARD (11)
// else by adding SN_RAMP_INC_ADVANCE (29)
// If there is no transition it is adjusted by SN_RAMP_INC (20)
#define SN_RAMP_INC (SN_RX_RAMP_LEN/SN_RX_SAMPLES_PER_BIT)
#define SN_RAMP_TRANSITION SN_RX_RAMP_LEN/2
#define SN_RAMP_ADJUST 9
#define SN_RAMP_INC_RETARD (SN_RAMP_INC-SN_RAMP_ADJUST)
#define SN_RAMP_INC_ADVANCE (SN_RAMP_INC+SN_RAMP_ADJUST)

#endif
// Outgoing message bits grouped as 6-bit words
// 36 alternating 1/0 bits, followed by 12 bits of start symbol
// Followed immediately by the 4-6 bit encoded byte count,
// message buffer and 2 byte FCS
// Each byte from the byte count on is translated into 2x6-bit words
// Caution, each symbol is transmitted LSBit first,
// but each byte is transmitted high nybble first
#if defined(VW_RADIO)
#define SN_PREAMBLE_LEN 8
#elif defined(MW_RADIO)
#define SN_PREAMBLE_LEN 4
#else
#error no radio module defined
#endif

// Initialise the VirtualWire software, to operate at speed bits per second
// Call this one in your setup() after any sn_set_* calls
// Must call sn_rx_start() before you will get any messages
void sn_setup(void);

#ifdef SN_RX_CAPABLE
// Start the Phase Locked Loop listening to the receiver
// Must do this before you can receive any messages
// When a message is available (good checksum or not), sn_have_message();
// will return true.
void sn_rx_start(void);

// Stop the Phase Locked Loop listening to the receiver
// No messages will be received until sn_rx_start() is called again
// Saves interrupt processing cycles
void sn_rx_stop(void);

// Returns true if an unread message is available
uint8_t sn_have_message(void);

// If a message is available (good checksum or not), copies
// up to *len octets to buf.
// Returns true if there was a message and the checksum was good
uint8_t sn_get_message(uint8_t* buf, uint8_t* len);

// Block until a message is available
void sn_wait_rx(void);

#ifdef USE_MILLIS
// or for a max time
uint8_t sn_wait_rx_max(unsigned long milliseconds);
#endif
#endif

#ifdef SN_TX_CAPABLE
// Send a message with the given length. Returns almost immediately,
// and message will be sent at the right timing by interrupts
// Returns true if the message was accepted for transmissions
// Returns false if the message is too long (>SN_MAX_MESSAGE_LEN - 3)
uint8_t sn_send(uint8_t* buf, uint8_t len);
uint8_t sn_send_wait(uint8_t* buf, uint8_t len);

// Return true if the transmitter is active
uint8_t sn_tx_active(void);

// Block until the transmitter is idle
void sn_wait_tx(void);
#endif

#ifdef USE_MILLIS
unsigned long millis(void);
#define ISR_PER_MILLI (SN_SPEED*SN_RX_SAMPLES_PER_BIT/1000)
#endif

#ifdef USE_WDT
unsigned long seconds(void);
void wd_sleep(void);
#endif
uint8_t read_adc(uint8_t mux);

void dbg_tx(const char *msg);
#endif /* SENSORNET_H_ */
