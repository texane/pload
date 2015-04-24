CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -O2 -I.

SRCS := main.c serial.c
OBJS := $(SRCS:.c=.o)

all: a.out

a.out:	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm $(OBJS)

fclean:	clean
	-rm a.out

.PHONY: all clean fclean
