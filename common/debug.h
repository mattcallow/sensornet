#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef __AVR__
#include "debug_avr.h"
#endif

void _dbg_u8h(uint8_t value);
void _dbg_u16h(uint16_t value);
void _dbg_u32h(uint32_t value);
void _dbg_strP(const char *str);
void _dbg_strP_u8h_nl(const char *str, uint8_t value);
void _dbg_strP_u16h_nl(const char *str, uint16_t value);
void _dbg_strP_u32h_nl(const char *str, uint32_t value);

#ifdef SERIAL_DEBUG
#define DBG_U8H(v)		_dbg_u8h(v)
#define DBG_U16H(v)		_dbg_u16h(v)
#define DBG_U32H(v)		_dbg_u32h(v)
#define DBG_STRP(s) 		_dbg_strP(PSTR(s))
#define DBG_STRP_U8H_NL(s, v)	_dbg_strP_u8h_nl(PSTR(s), v)
#define DBG_STRP_U16H_NL(s, v)	_dbg_strP_u16h_nl(PSTR(s), v)
#define DBG_STRP_U32H_NL(s, v)	_dbg_strP_u32h_nl(PSTR(s), v)
#else
#define DBG_U8H(v)		(void)(v)
#define DBG_U16H(v)		(void)(v)
#define DBG_U32H(v)		(void)(v)
#define DBG_STRP(s) 		(void)(s)
#define DBG_STRP_U8H_NL(s, v)	(void)(s); (void)(v)
#define DBG_STRP_U16H_NL(s, v)	(void)(s); (void)(v)
#define DBG_STRP_U32H_NL(s, v)	(void)(s); (void)(v)
#endif // SERIAL_DEBUG

#endif //
// vim: ts=4 sw=4
