MANDATORY_CFLAGS := -std=c99
CFLAGS ?= -g3 -O2
CC := gcc

DUMPROTATE_O := dumprotate.o args.o config.o dr_main.o dr_help.o ssize2bytes.o


.PHONY: all clean clean_daemon

all: dumprot

clean: clean_daemon

clean_daemon:
	$(RM) -f $(DUMPROTATE_O) dumprotate

dumprot: $(DUMPROTATE_O)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(MANDATORY_CFLAGS) $(CFLAGS) -c -o $@ $<

