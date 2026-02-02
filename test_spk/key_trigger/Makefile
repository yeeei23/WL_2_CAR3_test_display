CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -O2

key_trigger: trigger.c trigger_send.c
	$(CC) $(CFLAGS) -o key_trigger trigger.c trigger_send.c

clean:
	rm -f key_trigger

.PHONY: clean
