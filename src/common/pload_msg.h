#ifndef COMMON_PLOAD_MSG_H_INCLUDED
#define COMMON_PLOAD_MSG_H_INCLUDED


#include <stdint.h>


#define PLOAD_MSG_OP_SET_STEPS 0x00

#define PLOAD_SYNC_BYTE 0xa5
#define PLOAD_SYNC_END 0x5a

/* set_steps ops */
#define PLOAD_STEP_OP_CONST 0x00
#define PLOAD_STEP_OP_RAMP 0x01
#define PLOAD_STEP_OP_REPEAT 0x02

/* generator main clock frequency */
#define PLOAD_FCLOCK 1000000


typedef struct pload_msg
{
  /* warning: everything must be packed attribtued */

  uint8_t op;

  union
  {
    struct
    {
      /* set the current generator configuration */

      /* current intensity values, in mA */
#define PLOAD_STEP_COUNT 32
      uint8_t op[PLOAD_STEP_COUNT];
      uint32_t arg0[PLOAD_STEP_COUNT];
      uint32_t arg1[PLOAD_STEP_COUNT];
      uint8_t count;

    } __attribute__((packed)) steps;

  } __attribute__((packed)) u;

  /* 0xff if synchronization wanted */
  uint8_t sync;

} __attribute__((packed)) pload_msg_t;


#endif /* COMMON_PLOAD_MSG_H_INCLUDED */
