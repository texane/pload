#include <stdint.h>
#include "mk20dx128.h"
#include "serial.c"

/* use internal 1.2V vref */
#define DAC_USE_VREF
#include "dac.c"

#ifdef DAC_USE_VREF
#include "vref.c"
#endif /* DAC_USE_VREF */


extern void delay(uint32_t);


/* main */

static uint32_t ma_to_dac(uint32_t ma)
{
  /* ma the milliamps */

  /* do the full computation here to avoid truncatures */

  /* we have */
  /* mv = ma * rsense = ma * 0.05 = ma / 20 */
  /* and */
  /* dac_value = mv * 4096 / DAC_VREF_MV */
  /* thus */
  /* dac_value = (ma * 4096) / (DAC_VREF_MV * 20) */

  /* also, a factor of 8 because of voltage divider */

  return (ma * 4096 * 8) / (DAC_VREF_MV * 20);
}

int main(void)
{
#ifdef DAC_USE_VREF
  vref_setup();
#endif /* DAC_USE_VREF */

  dac_setup();
  dac_enable();

  serial_setup();
  SERIAL_WRITE_STRING("starting\r\n");

  while (1)
  {
    SERIAL_WRITE_STRING("0\r\n");
    dac_set(ma_to_dac(0));
    delay(1000);

    SERIAL_WRITE_STRING("10\r\n");
    dac_set(ma_to_dac(10));
    delay(5000);

    SERIAL_WRITE_STRING("50\r\n");
    dac_set(ma_to_dac(50));
    delay(5000);

    SERIAL_WRITE_STRING("100\r\n");
    dac_set(ma_to_dac(100));
    delay(5000);

    SERIAL_WRITE_STRING("200\r\n");
    dac_set(ma_to_dac(200));
    delay(5000);

    SERIAL_WRITE_STRING("500\r\n");
    dac_set(ma_to_dac(500));
    delay(5000);

    SERIAL_WRITE_STRING("750\r\n");
    dac_set(ma_to_dac(750));
    delay(5000);

    SERIAL_WRITE_STRING("1000\r\n");
    dac_set(ma_to_dac(1000));
    delay(5000);
  }

  return 0;
}
