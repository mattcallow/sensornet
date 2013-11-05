// vwire.c
	// make sure that we have transmitted all data before sleeping
	// otherwise we may sleep mid transmit, leaving the transmitter enabled
//
// Virtual Wire implementation for Arduino
// See the README file in this directory for documentation
//
// Changes:
// 1.5 2008-05-25: fixed a bug that could prevent messages with certain
//  bytes sequences being received (false message start detected)
// 1.6 2011-09-10: Patch from David Bath to prevent unconditional reenabling of the receiver
//  at end of transmission.
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2008 Mike McCauley
// $Id: VirtualWire.cpp,v 1.6 2012/01/10 22:21:03 mikem Exp mikem $

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


#ifdef SN_TX_CAPABLE
static uint8_t sn_tx_buf[SN_MAX_MESSAGE_LEN + SN_PREAMBLE_LEN] 
     = {0x55, 0x55, 0x00, 0xff};

void sn_tx_start(void);
// Number of symbols in sn_tx_buf to be sent;
static uint8_t sn_tx_len = 0;

// Index of the next symbol to send. Ranges from 0 to sn_tx_len
static uint8_t sn_tx_index = 0;

// Number of next half bit to send
static uint8_t sn_tx_half_bit = 0;

// Flag to indicated the transmitter is active
static volatile uint8_t sn_tx_enabled = 0;

// Total number of messages sent
static uint16_t sn_tx_msg_count = 0;
#endif


#ifdef SN_RX_CAPABLE
// Current receiver sample
static uint8_t sn_rx_sample = 0;

// Last receiver sample
static uint8_t sn_rx_last_sample = 0;

// PLL ramp, varies between 0 and SN_RX_RAMP_LEN-1 (159) over 
// SN_RX_SAMPLES_PER_BIT (8) samples per nominal bit time. 
// When the PLL is synchronised, bit transitions happen at about the
// 0 mark. 
static uint8_t sn_rx_pll_ramp = 0;

// This is the integrate and dump integral. If there are <5 0 samples in the PLL cycle
// the bit is declared a 0, else a 1
static uint8_t sn_rx_integrator = 0;

// Flag indictate if we have seen the start symbol of a new message and are
// in the processes of reading and decoding it
static uint8_t sn_rx_active = 0;

// Flag to indicate that a new message is available
static volatile uint8_t sn_rx_done = 0;

// Flag to indicate the receiver PLL is to run
static uint8_t sn_rx_enabled = 0;

// Last 12 bits received, so we can look for the start symbol
static uint16_t sn_rx_bits = 0;

// How many bits of message we have received. Ranges from 0 to 12
static uint8_t sn_rx_bit_count = 0;

// The incoming message buffer
static uint8_t sn_rx_buf[SN_MAX_MESSAGE_LEN];

// The incoming message expected length
static uint8_t sn_rx_count = 0;

// The incoming message buffer length received so far
static volatile uint8_t sn_rx_len = 0;

// Number of bad messages received and dropped due to bad lengths
static uint8_t sn_rx_bad = 0;

// Number of good messages received
static uint8_t sn_rx_good = 0;

#endif

#ifdef SN_RX_CAPABLE
// Called 8 times per bit period
// Phase locked loop tries to synchronise with the transmitter so that bit 
// transitions occur at about the time sn_rx_pll_ramp is 0;
// Then the average is computed over each bit period to deduce the bit value
void sn_pll(void)
{
    // Integrate each sample
    if (sn_rx_sample)
    	sn_rx_integrator++;

    if (sn_rx_sample != sn_rx_last_sample)
    {
	// Transition, advance if ramp > 80, retard if < 80
	sn_rx_pll_ramp += ((sn_rx_pll_ramp < SN_RAMP_TRANSITION) 
			   ? SN_RAMP_INC_RETARD 
			   : SN_RAMP_INC_ADVANCE);
	sn_rx_last_sample = sn_rx_sample;
    }
    else
    {
	// No transition
	// Advance ramp by standard 20 (== 160/8 samples)
	sn_rx_pll_ramp += SN_RAMP_INC;
    }
    if (sn_rx_pll_ramp >= SN_RX_RAMP_LEN)
    {
	// Add this to the 12th bit of sn_rx_bits, LSB first
	// The last 12 bits are kept
	sn_rx_bits >>= 1;

	// Check the integrator to see how many samples in this cycle were high.
	// If < 5 out of 8, then its declared a 0 bit, else a 1;
	if (sn_rx_integrator >= 5)
	    sn_rx_bits |= 0x800;

	sn_rx_pll_ramp -= SN_RX_RAMP_LEN;
	sn_rx_integrator = 0; // Clear the integral for the next cycle

	if (sn_rx_active)
	{
	    // We have the start symbol and now we are collecting message bits,
	    // 6 per symbol, each which has to be decoded to 4 bits
	    if (++sn_rx_bit_count >= 12)
	    {
		// Have 12 bits of encoded message == 1 byte encoded
		// Decode as 2 lots of 6 bits into 2 lots of 4 bits
		// The 6 lsbits are the high nybble
		uint8_t this_byte = 
		    (sn_symbol_6to4(sn_rx_bits & 0x3f)) << 4 
		    | sn_symbol_6to4(sn_rx_bits >> 6);

		// The first decoded byte is the byte count of the following message
		// the count includes the byte count and the 2 trailing FCS bytes
		// REVISIT: may also include the ACK flag at 0x40
		if (sn_rx_len == 0)
		{
		    // The first byte is the byte count
		    // Check it for sensibility. It cant be less than 4, since it
		    // includes the bytes count itself and the 2 byte FCS
		    sn_rx_count = this_byte;
		    if (sn_rx_count < 4 || sn_rx_count > SN_MAX_MESSAGE_LEN)
		    {
			// Stupid message length, drop the whole thing
			sn_rx_active = false;
			sn_rx_bad++;
                        return;
		    }
		}
		sn_rx_buf[sn_rx_len++] = this_byte;

		if (sn_rx_len >= sn_rx_count)
		{
		    // Got all the bytes now
		    sn_rx_active = false;
		    sn_rx_good++;
		    sn_rx_done = true; // Better come get it before the next one starts
		}
		sn_rx_bit_count = 0;
	    }
	}
	// Not in a message, see if we have a start symbol
	else if (sn_rx_bits == 0xb38)
	{
	    // Have start symbol, start collecting message
	    sn_rx_active = true;
	    sn_rx_bit_count = 0;
	    sn_rx_len = 0;
	    sn_rx_done = false; // Too bad if you missed the last message
	}
    }
}
#endif

void sn_setup(void)
{
#ifdef __AVR_ATtiny85__
    TCCR0A = _BV(WGM01);
#if ((F_CPU == 8000000) || (F_CPU == 4000000))
	#if ((SN_SPEED) < 4000)
        TCCR0B = _BV(CS01) | _BV(CS00); // div64 prescale
		#define SN_TIMER_PRESCALE 64UL
	#elif ((SN_SPEED) < 32000)
        TCCR0B = _BV(CS01); // div8 prescale
		#define SN_TIMER_PRESCALE 8UL
	#else
        TCCR0B = _BV(CS00); // no prescale
		#define SN_TIMER_PRESCALE 1UL
	#endif
#elif (F_CPU == 1000000)
	#if ((SN_SPEED*SN_RX_SAMPLES_PER_BIT) < 4000)
        TCCR0B = _BV(CS01); // div8 prescale
		#define SN_TIMER_PRESCALE 8UL
	#else
        TCCR0B = _BV(CS00); // no prescale
		#define SN_TIMER_PRESCALE 1UL
	#endif
#else
	#error Invalid F_CPU. Only 1MHz, 4Mhz, and 8Mhz supported
#endif // ATTiny85

#define _O_VAL_  ((F_CPU/SN_TIMER_PRESCALE) / (SN_SPEED))
#if (_O_VAL_ > 255)
#error timer overflow
#endif
    OCR0A = _O_VAL_;
    TIMSK |= _BV(OCIE0A);
#else
#error unsupported CPU
#endif
#ifdef SN_RX_CAPABLE
    DDR(RX_PORT) &= ~RX_MASK; // set as input
#endif
#ifdef SN_TX_CAPABLE
    DDR(TX_PORT) |= TX_MASK; // set as output
#endif
    // global interrupt enable
    sei();
}
#ifdef SN_TX_CAPABLE
// Start the transmitter, call when the tx buffer is ready to go and sn_tx_len is
// set to the total number of symbols to send
void sn_tx_start(void)
{
    sn_tx_index = 0;
    sn_tx_half_bit = 0;

#ifdef SN_RX_CAPABLE
    // Disable the receiver 
    sn_rx_enabled = false;
#endif
    // Next tick interrupt will send the first bit
    sn_tx_enabled = true;
}

// Stop the transmitter, call when all bits are sent
void sn_tx_stop(void)
{
    // No more ticks for the transmitter
    sn_tx_enabled = false;
    // Disable the transmitter hardware
    PORT(TX_PORT) &= ~TX_MASK;
#ifdef SN_RX_CAPABLE
    // Enable the receiver 
    sn_rx_enabled = true;
#endif
}
#endif

#ifdef SN_RX_CAPABLE
// Enable the receiver. When a message becomes available, sn_rx_done flag
// is set, and sn_wait_rx() will return.
void sn_rx_start()
{
    if (!sn_rx_enabled)
    {
	sn_rx_enabled = true;
	sn_rx_active = false; // Never restart a partial message
    }
}

// Disable the receiver
void sn_rx_stop()
{
    sn_rx_enabled = false;
}
#endif
#ifdef SN_TX_CAPABLE
// Return true if the transmitter is active
uint8_t sn_tx_active()
{
    return sn_tx_enabled;
}

// Wait for the transmitter to become available
// Busy-wait loop until the ISR says the message has been sent
void sn_wait_tx()
{
    while (sn_tx_enabled)
	;
}
#endif
#ifdef SN_RX_CAPABLE
// Wait for the receiver to get a message
// Busy-wait loop until the ISR says a message is available
// can then call sn_get_message()
void sn_wait_rx()
{
    while (!sn_rx_done)
	;
}

#ifdef USE_MILLIS
// Wait at most max milliseconds for the receiver to receive a message
// Return the truth of whether there is a message
uint8_t sn_wait_rx_max(unsigned long milliseconds)
{
    unsigned long start = millis();

    while (!sn_rx_done && ((millis() - start) < milliseconds))
	;
    return sn_rx_done;
}
#endif
#endif

#ifdef SN_TX_CAPABLE
uint8_t sn_send_wait(uint8_t* buf, uint8_t len)
{
#ifdef LED_PORT
	PORT(LED_PORT) |= LED_MASK; // turn on
#endif
	uint8_t ret = sn_send(buf, len);
	sn_wait_tx();
#ifdef LED_PORT
	PORT(LED_PORT) &= ~LED_MASK; // turn off
#endif
	return ret;
}
// Wait until transmitter is available and encode and queue the message
// into sn_tx_buf
// The message is raw bytes, with no packet structure imposed
// It is transmitted preceded a byte count and followed by 2 FCS bytes
uint8_t sn_send(uint8_t* buf, uint8_t len)
{
    uint8_t i;
    uint8_t index = 0;
    uint16_t crc = 0xffff;
    uint8_t *p = sn_tx_buf + SN_PREAMBLE_LEN; // start of the message area
    uint8_t count = len + 3; // Added byte count and FCS to get total number of bytes

    if (len > SN_MAX_PAYLOAD)
	return false;

    // Wait for transmitter to become available
    sn_wait_tx();

    // Store message length
    crc = _crc_ccitt_update(crc, count);
    p[index++] = count;

    // Store the message
    for (i = 0; i < len; i++)
    {
	crc = _crc_ccitt_update(crc, buf[i]);
	p[index++] = buf[i];
    }

    // Append the fcs, 16 bits 
    // Caution: VW expects the _ones_complement_ of the CCITT CRC-16 as the FCS
    // VW sends FCS as low byte then hi byte
    crc = ~crc;
    p[index++] = crc & 0xff;
    p[index++] = (crc >> 8)  & 0xff;

    // Total number of bytes
    sn_tx_len = index + SN_PREAMBLE_LEN;

    // Start the low level interrupt handler sending symbols
    sn_tx_start();

    return true;
}

#endif
// This is the interrupt service routine called when timer1 overflows
// Its job is to output the next half bit from the transmitter 
ISR(TIM0_COMPA_vect)
{
#ifdef LED_PORT
	//PORT(LED_PORT) |= LED_MASK; // turn on
#endif
#ifdef USE_MILLIS
#error TODO - timing will be wrong
	if (f_ms++ >= ISR_PER_MILLI)
	{
		ms++;
		f_ms=0;
	}
#endif
#ifdef SN_TX_CAPABLE
   if (sn_tx_enabled) 
	{
		// Send next half bit
		// Symbols are sent MSB first
		// Finished sending the whole message? (after waiting one bit period 
		// since the last bit)
		if (sn_tx_index >= sn_tx_len && (sn_tx_half_bit & 1))
		{
			sn_tx_stop();
			sn_tx_msg_count++;
		}
		else
		{
			if ((sn_tx_buf[sn_tx_index] & (0x80 >> (sn_tx_half_bit/2)))) {
				// One 
				if (sn_tx_half_bit&1) PORT(TX_PORT) |= TX_MASK;
				else PORT(TX_PORT) &= ~TX_MASK;
			} else {
				// Zero 
				if (sn_tx_half_bit&1) PORT(TX_PORT) &= ~TX_MASK;
				else PORT(TX_PORT) |= TX_MASK;
			}
			if (sn_tx_half_bit++ >= 15)
			{
				sn_tx_half_bit = 0;
				sn_tx_index++;
			}
		}
	}
#endif
#ifdef LED_PORT
	//PORT(LED_PORT) &= ~LED_MASK; // turn off
#endif
}

#ifdef SN_RX_CAPABLE
// Return true if there is a message available
uint8_t sn_have_message()
{
    return sn_rx_done;
}

// Get the last message received (without byte count or FCS)
// Copy at most *len bytes, set *len to the actual number copied
// Return true if there is a message and the FCS is OK
uint8_t sn_get_message(uint8_t* buf, uint8_t* len)
{
    uint8_t rxlen;

    // Message available?
    if (!sn_rx_done)
	return false;

    // Wait until sn_rx_done is set before reading sn_rx_len
    // then remove bytecount and FCS
    rxlen = sn_rx_len - 3;

    // Copy message (good or bad)
    if (*len > rxlen)
	*len = rxlen;
    memcpy(buf, sn_rx_buf + 1, *len);

    sn_rx_done = false; // OK, got that message thanks

    // Check the FCS, return goodness
    return (sn_crc(sn_rx_buf, sn_rx_len) == 0xf0b8); // FCS OK?
}
#endif
/*
 * vim: ts=4 sw=4
 */
