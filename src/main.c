#include <stdint.h>
#include "mk20dx128.h"
#include "serial.c"

/* use internal 1.2V vref */
#define DAC_USE_VREF
#include "dac.c"

#include "vref.c"


extern void delay(uint32_t);


/* main */

int main(void)
{
  vref_setup();
  dac_setup();

  serial_setup();
  SERIAL_WRITE_STRING("starting\r\n");

  dac_enable();

  while (1)
  {
    SERIAL_WRITE_STRING("alive\r\n");

    /* warning: must be le 0xfff */
#if 0
    dac_set(4096 / 1 - 1);
    delay(500);
    dac_set(4096 / 2);
    delay(500);
    dac_set(4096 / 4);
    delay(500);
    dac_set(4096 / 8);
    delay(500);
    dac_set(4096 / 16);
    delay(500);
    dac_set(4096 / 32);
    delay(500);
    dac_set(4096 / 64);
    delay(500);
#endif

    dac_set(dac_mv_to_val(0));
    delay(5000);

    dac_set(dac_mv_to_val(10));
    delay(5000);

    dac_set(dac_mv_to_val(50));
    delay(5000);

    dac_set(dac_mv_to_val(100));
    delay(5000);

    dac_set(dac_mv_to_val(150));
    delay(5000);

    dac_set(dac_mv_to_val(1000));
    delay(5000);
  }

  return 0;
}
