#ifndef COMMON_PLOAD_MSG_H_INCLUDED
#define COMMON_PLOAD_MSG_H_INCLUDED


#include <stdint.h>


#define PLOAD_OP_GET_INFO 0x00
#define PLOAD_OP_SET_CONF 0x01


#define PLOAD_SYNC_BYTE 0xa5
#define PLOAD_SYNC_END 0x5a


typedef struct pload_msg
{
  /* warning: everything must be packed attribtued */

  uint8_t op;

  union
  {
    struct
    {
      /* get the device information */

      /* clock frequency, in Hz */
      uint32_t clock_freq;

    } __attribute__((packed)) info;

    struct
    {
      /* set the current generator configuration */

      /* current intensity values, in mA */
#define PLOAD_CUR_NVAL 32
      uint16_t cur_vals[PLOAD_CUR_NVAL];
      uint8_t cur_nval;

      /* clock frequency divider */
      uint32_t clock_fdiv;

    } __attribute__((packed)) conf;

  } __attribute__((packed)) u;

  /* 0xff if synchronization wanted */
  uint8_t sync;

} __attribute__((packed)) pload_msg_t;


#endif /* COMMON_PLOAD_MSG_H_INCLUDED */
