#ifndef PLOAD_COMMON_H_INCLUDED
#define PLOAD_COMMON_H_INCLUDED


#include <stdint.h>


#define PLOAD_MSG_OP_SET_STEPS 0x00
#define PLOAD_MSG_OP_SYNC 0xa5

/* set_steps ops */
#define PLOAD_STEP_OP_CONST 0x00
#define PLOAD_STEP_OP_RAMP 0x01
#define PLOAD_STEP_OP_REPEAT 0x02

/* generator main clock frequency */
#define PLOAD_CLOCK_FREQ 1000

/* maximum load current, in mA */
#define PLOAD_MAX_CURRENT 1500


typedef struct pload_msg
{
  /* note: little endian format */
  /* warning: everything must be packed attribtued */

  uint32_t op;

  union
  {
    struct
    {
      /* set the current generator configuration */

      /* current intensity values, in mA */
#define PLOAD_STEP_COUNT 32
      uint8_t op[PLOAD_STEP_COUNT];
      int32_t arg0[PLOAD_STEP_COUNT];
      int32_t arg1[PLOAD_STEP_COUNT];
      uint8_t count;
    } __attribute__((packed)) steps;

  } __attribute__((packed)) u;

} __attribute__((packed)) pload_msg_t;


#endif /* PLOAD_COMMON_H_INCLUDED */
