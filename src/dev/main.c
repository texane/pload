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


static uint32_t ma_to_dac(uint32_t ma)
{
  /* convert ma, the current in mA, into a DAC value */

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


/* pload sequencer */

/* when a full message is got in pload_msg, the main routine */
/* starts the sequencer by actually enabling pit1. when this */
/* is done, pit1_isr is called at PLOAD_CLOCK_FREQ Hz. It  */
/* executes the step sequence of the message. pload_step_index */
/* tracks the current step. the sequence is repeated as described */
/* in the message, and tracked by pload_repeat_index. each step */
/* is active for a tick count, tracked by pload_tick_count. */
/* as the sequencer executes, it updates the current value with */
/* the dac. pload_current tracks of the current value. */

static volatile uint32_t pload_flags = 0;
static volatile pload_msg_t pload_msg;
static volatile uint32_t pload_step_index = 0;
static volatile uint32_t pload_tick_count = 0;
static volatile uint32_t pload_repeat_index = 0;
static volatile int32_t pload_current = 0;

void pit1_isr(void)
{
  if (--pload_tick_count)
  {
    /* continue current step */

    if (pload_msg.u.steps.op[pload_step_index] == PLOAD_STEP_OP_RAMP)
    {
      pload_current += pload_msg.u.steps.arg0[pload_step_index];
      dac_set(ma_to_dac((uint32_t)pload_current));

#if 0 /* DEBUG */
      SERIAL_WRITE_STRING("dac_set:");
      serial_write(uint32_to_string((uint32_t)pload_current), 8);
      SERIAL_WRITE_STRING("\r\n");
#endif /* DEBUG */
    }

    goto on_done;
  }

  if ((++pload_step_index) == (uint32_t)pload_msg.u.steps.count)
  {
#if 0 /* DEBUG */
    SERIAL_WRITE_STRING("stopping\r\n");
#endif /* DEBUG */

    pit_stop(1);
    goto on_done;
  }

  /* current step done, load next state */

  switch (pload_msg.u.steps.op[pload_step_index])
  {
  case PLOAD_STEP_OP_CONST:
    {
    const_case:
      pload_current = (int32_t)pload_msg.u.steps.arg0[pload_step_index];
      dac_set(ma_to_dac((uint32_t)pload_current));

#if 0 /* DEBUG */
      SERIAL_WRITE_STRING("dac_set:");
      serial_write(uint32_to_string((uint32_t)pload_current), 8);
      SERIAL_WRITE_STRING("\r\n");
#endif /* DEBUG */

      pload_tick_count = (uint32_t)pload_msg.u.steps.arg1[pload_step_index];
      break ;
    }

  case PLOAD_STEP_OP_RAMP:
    {
      pload_current += (int32_t)pload_msg.u.steps.arg0[pload_step_index];
      dac_set(ma_to_dac((uint32_t)pload_current));

#if 0 /* DEBUG */
      SERIAL_WRITE_STRING("dac_set:");
      serial_write(uint32_to_string((uint32_t)pload_current), 8);
      SERIAL_WRITE_STRING("\r\n");
#endif /* DEBUG */

      pload_tick_count = (uint32_t)pload_msg.u.steps.arg1[pload_step_index];
      break ;
    }

  case PLOAD_STEP_OP_REPEAT:
    {
      const int32_t repeat_count = pload_msg.u.steps.arg0[pload_step_index];

      if ((++pload_repeat_index) == (uint32_t)repeat_count)
      {
#if 0 /* DEBUG */
	SERIAL_WRITE_STRING("stopping\r\n");
#endif /* DEBUG */

	pit_stop(1);
	goto on_done;
      }

      if (repeat_count == -1)
      {
	pload_repeat_index = 0;
      }

      /* next step */
      pload_step_index = 0;
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
    if (rsize > (sizeof(pload_msg) - msize)) rsize = sizeof(pload_msg) - msize;
    if (rsize == 0) continue ;

    /* stop the generator if active */

    if (pload_flags & (1 << 0))
    {
#if 0 /* DEBUG */
      SERIAL_WRITE_STRING("stopping\r\n");
#endif /* DEBUG */

      pload_flags &= ~(1 << 0);
      pit_stop(1);
    }

    serial_read((uint8_t*)&pload_msg + msize, rsize);
    msize += rsize;
    if (msize != sizeof(pload_msg)) continue ;

    /* new message */
    msize = 0;

    if (pload_msg.op != PLOAD_MSG_OP_SET_STEPS) continue ;

    /* start the generator */

    if ((pload_flags & (1 << 0)) == 0)
    {
#if 0 /* DEBUG */
      SERIAL_WRITE_STRING("starting\r\n");
#endif /* DEBUG */

      pload_step_index = 0;
      pload_tick_count = (uint32_t)pload_msg.u.steps.arg1[0];
      pload_repeat_index = 0;
      pload_current = (int32_t)pload_msg.u.steps.arg0[0];

      dac_set(ma_to_dac((uint32_t)pload_current));

#if 0 /* DEBUG */
      SERIAL_WRITE_STRING("dac_set:");
      serial_write(uint32_to_string((uint32_t)pload_current), 8);
      SERIAL_WRITE_STRING("\r\n");
#endif /* DEBUG */

      pload_flags |= 1 << 0;
      pit_start(1, F_BUS / PLOAD_CLOCK_FREQ);
    }
  }

  return 0;
}
