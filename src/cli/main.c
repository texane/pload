/* pload commnunication client */

/* description */
/* Describe a current profile to generate. A profile is */
/* specified by using an ordered sequence of steps. Steps */
/* are made of const, ramp and repeat keywords, along with */
/* the corresponding arguments. The step count is currently */
/* limited to 32. */

/* usage */
/* ./a.out */
/*  -const i us */
/*  -ramp i us */
/*  -repeat n */

/* example: generate 20mA during 100us and ramp to 250mA. The */
/* ramp duration is 200us. Stay at 250mA for 1ms. repeat 3 times */
/* ./a.out -const 20 100 -ramp 250 200 -const 250 1000 -repeat 3 */

/* example: generate 100mA during 100us and ramp to 50mA. Stay */
/* at 50mA for 200us. Ramp duration is 40us. Repeat forever. */
/* ./a.out -const 100 100 -ramp 50 40 -const 50 200 -repeat -1 */

/* example: continuously generate 100mA for 10ms then 200mA for */
/* 20ms. */
/* ./a.out -const 100 10000 -const 200 20000 -repeat -1 */


#include "../common/pload_msg.h"


#define CONFIG_DEBUG 1
#if CONFIG_DEBUG
#include <stdio.h>
#define PERROR()					\
do {							\
printf("[!] %s, %u\n", __FILE__, __LINE__);		\
} while (0)
#else
#define PERROR()
#endif


typedef struct pload_handle
{
} pload_handle_t;


static int pload_open(pload_handle_t* pload)
{
  return -1;
}

static void pload_close(pload_handle_t* pload)
{
}

static int pload_sync(pload_handle_t* pload)
{
#if 0
  static const uint8_t sync_byte = SNRF_SYNC_BYTE;
  static const uint8_t end_byte = SNRF_SYNC_END;

  size_t i;

  for (i = 0; i < (4 * sizeof(snrf_msg_t)); ++i)
  {
    usleep(100);
    if (serial_writen(&snrf->serial, &sync_byte, 1))
    {
      SNRF_PERROR();
      return -1;
    }
  }

  /* delay required beftore flushing and writing end byte */
  usleep(10000);

  if (serial_flush_txrx(&snrf->serial))
  {
    SNRF_PERROR();
    return -1;
  }

  if (serial_writen(&snrf->serial, &end_byte, 1))
  {
    SNRF_PERROR();
    return -1;
  }

  snrf->state = SNRF_STATE_CONF;
#endif

  return 0;
}

static int pload_send_msg
(pload_handle_t* pload, const pload_msg_t* msg)
{
  return -1;
}

int main(int ac, char** av)
{
  return 0;
}
