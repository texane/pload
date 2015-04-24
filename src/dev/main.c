#include <stdint.h>
#include "mk20dx128.h"
#include "serial.c"
#include "pit.c"
#include "../common/pload_common.h"

/* use internal 1.2V vref */
#define DAC_USE_VREF
#include "dac.c"

#ifdef DAC_USE_VREF
#include "vref.c"
#endif /* DAC_USE_VREF */


extern void delay(uint32_t);


/* ma the milliamps conversion */

static uint32_t ma_to_dac(uint32_t ma)
{
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


/* pit interrupt handler */

static volatile uint32_t pit_flags = 0;
static volatile pload_msg_t msg;
static volatile uint32_t step_index = 0;
static volatile uint32_t tick_count = 0;
static volatile uint32_t repeat_index = 0;
static volatile int32_t current_val = 0;

void pit1_isr(void)
{
  if (--tick_count)
  {
    /* continue current step */

    if (msg.u.steps.op[step_index] == PLOAD_STEP_OP_RAMP)
    {
      current_val += msg.u.steps.arg0[step_index];
      dac_set(ma_to_dac((uint32_t)current_val));

#if 0 /* DEBUG */
      SERIAL_WRITE_STRING("dac_set:");
      serial_write(uint32_to_string((uint32_t)current_val), 8);
      SERIAL_WRITE_STRING("\r\n");
#endif /* DEBUG */
    }

    goto on_done;
  }

  if ((++step_index) == (uint32_t)msg.u.steps.count)
  {
#if 0 /* DEBUG */
    SERIAL_WRITE_STRING("stopping\r\n");
#endif /* DEBUG */

    pit_stop(1);
    goto on_done;
  }

  /* current step done, load next state */

  switch (msg.u.steps.op[step_index])
  {
  case PLOAD_STEP_OP_CONST:
    {
    const_case:
      current_val = (int32_t)msg.u.steps.arg0[step_index];
      dac_set(ma_to_dac((uint32_t)current_val));

#if 0 /* DEBUG */
      SERIAL_WRITE_STRING("dac_set:");
      serial_write(uint32_to_string((uint32_t)current_val), 8);
      SERIAL_WRITE_STRING("\r\n");
#endif /* DEBUG */

      tick_count = (uint32_t)msg.u.steps.arg1[step_index];
      break ;
    }

  case PLOAD_STEP_OP_RAMP:
    {
      current_val += (int32_t)msg.u.steps.arg0[step_index];
      dac_set(ma_to_dac((uint32_t)current_val));

#if 0 /* DEBUG */
      SERIAL_WRITE_STRING("dac_set:");
      serial_write(uint32_to_string((uint32_t)current_val), 8);
      SERIAL_WRITE_STRING("\r\n");
#endif /* DEBUG */

      tick_count = (uint32_t)msg.u.steps.arg1[step_index];
      break ;
    }

  case PLOAD_STEP_OP_REPEAT:
    {
      const int32_t repeat_count = msg.u.steps.arg0[step_index];

      if ((++repeat_index) == (uint32_t)repeat_count)
      {
#if 0 /* DEBUG */
	SERIAL_WRITE_STRING("stopping\r\n");
#endif /* DEBUG */

	pit_stop(1);
	goto on_done;
      }

      if (repeat_count == -1)
      {
	repeat_index = 0;
      }

      /* next step */
      step_index = 0;
      goto const_case;
      break ;
    }

  default: break ;
  }

 on_done:
  pit_clear_int(1);
}


/* main */

int main(void)
{
  uint32_t msize;
  uint32_t rsize;

#ifdef DAC_USE_VREF
  vref_setup();
#endif /* DAC_USE_VREF */

  dac_setup();
  dac_enable();

  serial_setup();

  msize = 0;

  while (1)
  {
    rsize = serial_get_rsize();
    if (rsize > (sizeof(msg) - msize)) rsize = sizeof(msg) - msize;
    if (rsize == 0) continue ;

    /* stop the generator if active */

    if (pit_flags & (1 << 0))
    {
#if 0 /* DEBUG */
      SERIAL_WRITE_STRING("stopping\r\n");
#endif /* DEBUG */

      pit_flags &= ~(1 << 0);
      pit_stop(1);
    }

    serial_read((uint8_t*)&msg + msize, rsize);
    msize += rsize;
    if (msize != sizeof(msg)) continue ;

    /* new message */
    msize = 0;

    if (msg.op != PLOAD_MSG_OP_SET_STEPS) continue ;

    /* start the generator */

    if ((pit_flags & (1 << 0)) == 0)
    {
#if 0 /* DEBUG */
      SERIAL_WRITE_STRING("starting\r\n");
#endif /* DEBUG */

      step_index = 0;
      tick_count = (uint32_t)msg.u.steps.arg1[0];
      repeat_index = 0;
      current_val = (int32_t)msg.u.steps.arg0[0];

      dac_set(ma_to_dac((uint32_t)current_val));

#if 0 /* DEBUG */
      SERIAL_WRITE_STRING("dac_set:");
      serial_write(uint32_to_string((uint32_t)current_val), 8);
      SERIAL_WRITE_STRING("\r\n");
#endif /* DEBUG */

      pit_flags |= 1 << 0;
      pit_start(1, F_BUS / PLOAD_CLOCK_FREQ);
    }
  }

  return 0;
}
