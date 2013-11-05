#include "debug.h"

void __dbg_init(void) __attribute__ ((naked)) __attribute__ ((section (".init3")));

void __dbg_init(void)
{
#ifdef SERIAL_DEBUG
#if (!defined(DBG_PORT) || !defined(DBG_BIT))
	#error "you must defne DBG_PORT and DBG_BIT"
#endif
	DDR(DBG_PORT) |= 1<<DBG_BIT;	// set as output
	PORT(DBG_PORT) |= 1<<DBG_BIT;	// set high
#endif
}


void _dbg_char(char value)
{
#ifdef SERIAL_DEBUG
#if (F_CPU == 1000000)
	dbg_bang_one_byte( value, PORT(DBG_PORT), DBG_BIT, 0b00100, 0b00010, 0, 28, 2 );
#elif (F_CPU == 8000000)
	// TODO - fix use of 0x18 insteab of PORTB
	// These settings are for 9600 baud at 8MHz
	dbg_bang_one_byte( value, 0x18, DBG_BIT, 0b01001, 0b00100, 3, 89, 0 );
#else
	dbg_invalid_f_cpu();
#endif
#endif
}

void _dbg_strP(const char *addr)
{
#ifdef SERIAL_DEBUG
	char value;
	while ((value = pgm_read_byte(addr++))) {
#if (F_CPU == 1000000)
		dbg_bang_one_byte( value, PORT(DBG_PORT), DBG_BIT, 0b00100, 0b00010, 0, 28, 2 );
#elif (F_CPU == 8000000)
		// TODO - fix use of 0x18 insteab of PORTB
		// These settings are for 9600 baud at 8MHz
		dbg_bang_one_byte( value, 0x18, DBG_BIT, 0b01001, 0b00100, 3, 89, 0 );
#else
		dbg_invalid_f_cpu();
#endif
	}
#endif
}

void _dbg_u8h(const uint8_t v)
{
	uint8_t l=v & 0x0f;
	uint8_t h=v >> 4;
	if (l>9) l+=55;
	else l+=48;
	if (h>9) h+=55;
	else h+=48;
	_dbg_char(h);
	_dbg_char(l);
}

void _dbg_u16h(const uint16_t v)
{
	uint8_t l=v & 0xff;
	uint8_t h=v >> 8;
	_dbg_u8h(h);
	_dbg_u8h(l);
}

void _dbg_u32h(const uint32_t v)
{
	uint16_t l=v & 0xffff;
	uint16_t h=v >> 16;
	_dbg_u16h(h);
	_dbg_u16h(l);
}

void _dbg_strP_u8h_nl(const char *str, uint8_t value)
{
	_dbg_strP(str);
	_dbg_u8h(value);
	_dbg_char('\r');
	_dbg_char('\n');
}

void _dbg_strP_u16h_nl(const char *str, uint16_t value)
{
	_dbg_strP(str);
	_dbg_u16h(value);
	_dbg_char('\r');
	_dbg_char('\n');
}

void _dbg_strP_u32h_nl(const char *str, uint32_t value)
{
	_dbg_strP(str);
	_dbg_u32h(value);
	_dbg_char('\r');
	_dbg_char('\n');
}
