#ifndef _MACROS_H_
#define _MACROS_H_

// utility macros from http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=73829
#define PORT_(port) PORT##port
#define DDR_(port)  DDR##port
#define PIN_(port)  PIN##port

#define PORT(port) PORT_(port)
#define DDR(port)  DDR_(port)
#define PIN(port)  PIN_(port)

#define SET_PIN_OUTPUT(port, bit) DDR(port) |= _BV(bit)
#define SET_PIN_INPUT(port, bit) DDR(port) &= ~_BV(bit)
#define SET_PIN_HI(port, bit) PORT(port) |= _BV(bit)
#define SET_PIN_LO(port, bit) PORT(port) &= ~_BV(bit)
#define READ_PIN(port, bit) (PIN(port) & _BV(bit))

#ifdef __AVR__
#define sn_delay_ms(x) _delay_ms(x)
#define sn_delay_us(x) _delay_us(x)
#include <util/delay.h>
#endif
#ifdef __PIC__
#define sn_delay_ms(x) __delay_ms(x)
#endif

#endif
