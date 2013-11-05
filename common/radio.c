#include <util/crc16.h>
#include "sensornet.h"
// Compute CRC over count bytes.
// This should only be ever called at user level, not interrupt level
uint16_t sn_crc(uint8_t *ptr, uint8_t count)
{
    uint16_t crc = 0xffff;

    while (count-- > 0) 
	crc = _crc_ccitt_update(crc, *ptr++);
    return crc;
}

#if defined(VW_RADIO)
#include "radio_vw.c"
#elif defined(MW_RADIO)
#include "radio_mw.c"
#else
#error No radio module defined
#endif
