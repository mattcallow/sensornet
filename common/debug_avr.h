#ifndef _DEBUG_AVR_H_
#define _DEBUG_AVR_H_

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "macros.h"
#include "config.h"


void dbg_params_not_constant( void ) __attribute__((error("Serial (TinyDebugSerial) called with non constant params.")));
void dbg_invalid_f_cpi( void ) __attribute__((error("Serial (TinyDebugSerial) called with invalid F_CPU.")));


__attribute__((always_inline, unused)) static inline void dbg_bang_one_byte( uint8_t value, uint8_t SER_REG, uint8_t SER_BIT, uint8_t lom, uint8_t him, uint8_t oloops, uint8_t iloops, uint8_t nops )
{
  if ( __builtin_constant_p( SER_REG ) 
      && __builtin_constant_p( SER_BIT ) 
      && __builtin_constant_p( lom ) 
      && __builtin_constant_p( him )
      && __builtin_constant_p( oloops ) 
      && __builtin_constant_p( iloops ) 
      && __builtin_constant_p( nops ) )
  {
      uint8_t i;
      uint8_t j;
      uint8_t ol;
      uint8_t il;
      uint8_t b;  // Initialized to the low bits
      uint8_t hib;
      uint8_t m;
      
      b   = ((value << 1) & 0x1F);
      hib = ((value >> 4) & 0x1F) | 0x10;
      
      asm volatile
      (
        "ldi   %[j], 2"                           "\n\t"
        "ldi   %[i], 5"                           "\n\t"
        "ldi   %[m], %[lom]"                      "\n\t"

        // Note: 8 MHz, 9600 baud ---> disabling interrupts does not appear to be necessary

        "cli"                                     "\n\t"

        "rjmp  L%=ntop"                           "\n\t"

      "L%=btop: "
        "nop"                                     "\n\t"      // ---> 7
        "nop"                                     "\n\t"      //
        "nop"                                     "\n\t"      //
        "nop"                                     "\n\t"      //
        "nop"                                     "\n\t"      //
        "nop"                                     "\n\t"      //
        "nop"                                     "\n\t"      //

      "L%=ntop: "
        "ror   %[b]"                              "\n\t"      // ---> 1
        
        "brcs  L%=bxh"                            "\n\t"      // 1  (not taken) 
        "cbi   %[serreg], %[serbit]"              "\n\t"      // 2
        "rjmp  L%=bxz"                            "\n\t"      // 2 
        
      "L%=bxh: "                                              // 2  (taken) 
        "sbi   %[serreg], %[serbit]"              "\n\t"      // 2
        "nop"                                     "\n\t"      // 1 

                                                              // ---> 5
      "L%=bxz: "

        "ror   %[m]"                              "\n\t"      // ---> 3 or 4 
        "brcc  L%=bnoe"                           "\n\t"      //
        "nop"                                     "\n\t"      //
        "nop"                                     "\n\t"      //
      "L%=bnoe: "

                                                              // ---> 1
      ".if %[oloops] >= 1"                        "\n\t"      // if oloops >= 1 then...
        "ldi   %[ol], %[oloops]"                  "\n\t"      // 4*oloops + oloops*(3*iloops) or oloops*((3*iloops)+4)
      "L%=odelay: "                               "\n\t"
      ".endif"                                    "\n\t"
        "ldi   %[il], %[iloops]"                  "\n\t"      // if oloops == 0 then...
      "L%=idelay: "                               "\n\t"      // (3*iloops)
        "dec   %[il]"                             "\n\t"
        "brne  L%=idelay"                         "\n\t"
        "nop"                                     "\n\t"
      ".if %[oloops] >= 1"                        "\n\t"
        "dec   %[ol]"                             "\n\t"
        "brne  L%=odelay"                         "\n\t"
        "nop"                                     "\n\t"
      ".endif"                                    "\n\t"

      ".if %[nops] >= 1"                          "\n\t"
        "nop"                                     "\n\t"      //
      ".endif"                                    "\n\t"
      ".if %[nops] >= 2"                          "\n\t"
        "nop"                                     "\n\t"      //
      ".endif"                                    "\n\t"

        "dec   %[i]"                              "\n\t"      // ---> 3
        "brne  L%=btop"                           "\n\t"      //
        "nop"                                     "\n\t"      //

        "dec   %[j]"                              "\n\t"      // ---> 7
        "breq  L%=bfin"                           "\n\t"      //
        "ldi   %[i], 5"                           "\n\t"      //
        "mov   %[b], %[hib]"                      "\n\t"      //
        "ldi   %[m], %[him]"                      "\n\t"      //
        "rjmp  L%=ntop"                           "\n\t"      //

      "L%=bfin: "

        "sei"                                     "\n\t"
        : 
          [i] "=&r" ( i ),
          [j] "=&r" ( j ),
          [ol] "=&r" ( ol ),
          [il] "=&r" ( il ),
          [m] "=&r" ( m )
        : 
          [b] "r" ( b ),
          [hib] "r" ( hib ),
          [serreg] "I" ( SER_REG ),
          [serbit] "M" ( SER_BIT ),
          [lom] "M" ( lom ),
          [him] "M" ( him ),
          [oloops] "M" ( oloops ),
          [iloops] "M" ( iloops ),
          [nops] "M" ( nops )
        :
          "r31",
          "r30"
      );
  }
  else
  {
    dbg_params_not_constant();
  }
}

#endif
