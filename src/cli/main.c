/* pload client */

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


#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "serial.h"
#include "../common/pload_common.h"


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


/* command line parsing */

typedef struct
{
  const char* dev_path;

  unsigned int step_op[PLOAD_STEP_COUNT];
  unsigned int step_arg0[PLOAD_STEP_COUNT];
  unsigned int step_arg1[PLOAD_STEP_COUNT];
  size_t step_count;

} cmdline_info_t;

static int parse_cmdline(cmdline_info_t* info, const char** av)
{
  info->dev_path = "/dev/ttyACM0";
  info->step_count = 0;

  for (; *av != NULL; ++*av)
  {
    if (strcmp(*av, "-dev") == 0)
    {
      if ((++*av) == NULL)
      {
	PERROR();
	return -1;
      }

      info->dev_path = *av;

      continue ;
    }

    if (info->step_count == PLOAD_STEP_COUNT)
    {
      PERROR();
      return -1;
    }

    if (strcmp(*av, "-const") == 0)
    {
      info->step_op[info->step_count] = PLOAD_STEP_OP_CONST;

      if ((++*av) == NULL)
      {
	PERROR();
	return -1;
      }

      info->step_arg0[info->step_count] = (unsigned int)atoi(*av);

      if ((++*av) == NULL)
      {
	PERROR();
	return -1;
      }

      info->step_arg1[info->step_count] = (unsigned int)atoi(*av);
    }
    else if (strcmp(*av, "-ramp") == 0)
    {
      info->step_op[info->step_count] = PLOAD_STEP_OP_RAMP;

      if ((++*av) == NULL)
      {
	PERROR();
	return -1;
      }

      info->step_arg0[info->step_count] = (unsigned int)atoi(*av);

      if ((++*av) == NULL)
      {
	PERROR();
	return -1;
      }

      info->step_arg1[info->step_count] = (unsigned int)atoi(*av);
    }
    else if (strcmp(*av, "-repeat") == 0)
    {
      info->step_op[info->step_count] = PLOAD_STEP_OP_REPEAT;

      if ((++*av) == NULL)
      {
	PERROR();
	return -1;
      }

      info->step_arg0[info->step_count] = (unsigned int)atoi(*av);
    }
    else
    {
      PERROR();
      return -1;
    }

    ++info->step_count;
  }

  return 0;
}


/* pload routines */

typedef struct pload_handle
{
  serial_handle_t serial;
} pload_handle_t;


static int pload_sync(pload_handle_t* pload)
{
  static const uint8_t sync_byte = PLOAD_SYNC_BYTE;
  static const uint8_t end_byte = PLOAD_SYNC_END;

  size_t i;

  for (i = 0; i < (4 * sizeof(pload_msg_t)); ++i)
  {
    usleep(100);
    if (serial_writen(&pload->serial, &sync_byte, 1))
    {
      PERROR();
      return -1;
    }
  }

  /* delay required beftore flushing and writing end byte */
  usleep(10000);

  if (serial_flush_txrx(&pload->serial))
  {
    PERROR();
    return -1;
  }

  if (serial_writen(&pload->serial, &end_byte, 1))
  {
    PERROR();
    return -1;
  }

  return 0;
}

static int pload_open(pload_handle_t* pload, const char* dev_path)
{
  static const serial_conf_t conf = { 115200, 8, SERIAL_PARITY_DISABLED, 1 };

  if (serial_open(&pload->serial, dev_path))
  {
    PERROR();
    goto on_error_0;
  }

  if (serial_set_conf(&pload->serial, &conf))
  {
    PERROR();
    goto on_error_1;
  }

  if (pload_sync(pload))
  {
    PERROR();
    goto on_error_1;
  }

  return 0;

 on_error_1:
  serial_close(&pload->serial);
 on_error_0:
  return -1;
}

static void pload_close(pload_handle_t* pload)
{
  serial_close(&pload->serial);
}

static int pload_write_msg
(pload_handle_t* pload, const pload_msg_t* msg)
{
  if (serial_writen(&pload->serial, (const void*)msg, sizeof(*msg)))
  {
    PERROR();
    return -1;
  }

  return 0;
}


/* main */

static inline uint32_t uint32_to_le(uint32_t x)
{
  return x;
}

int main(int ac, char** av)
{
  cmdline_info_t info;
  pload_handle_t pload;
  pload_msg_t msg;
  size_t i;
  int err = -1;

  if (parse_cmdline(&info, (const char**)av + 1))
  {
    PERROR();
    goto on_error_0;
  }

  if (pload_open(&pload, info.dev_path))
  {
    PERROR();
    goto on_error_0;
  }

  msg.op = PLOAD_MSG_OP_SET_STEPS;
  msg.u.steps.count = (uint8_t)info.step_count;
  for (i = 0; i != info.step_count; ++i)
  {
    msg.u.steps.op[i] = (uint8_t)info.step_op[i];
    msg.u.steps.arg0[i] = uint32_to_le((uint32_t)info.step_arg0[i]);
    msg.u.steps.arg1[i] = uint32_to_le((uint32_t)info.step_arg1[i]);
  }

  if (pload_write_msg(&pload, &msg))
  {
    PERROR();
    goto on_error_1;
  }

  err = 0;
 on_error_1:
  pload_close(&pload);
 on_error_0:
  return err;
}
