.PHONY: first all clean install uninstall

SOURCES = main.c event.c dbus.c
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))
DEPS = $(patsubst %.c,%.dep,$(SOURCES))

SHELL = sh
CC = gcc
LD = gcc
INSTALL = install -v
RM = rm -f -v

CFLAGS = -O3 -Wall -fomit-frame-pointer `pkg-config --cflags dbus-1`
LDFLAGS =
LIBS = `pkg-config --libs dbus-1`

first: all

-include $(DEPS)

all:	jacklistenerd

jacklistenerd:	$(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

%.dep:  %.c Makefile
	$(SHELL) -ec "$(CC) -x c $(CFLAGS) -MM $< | sed -re 's|([^:]+.o)( *:+)|$(<:.c=.o) $@\2|;'" > $@

%.o:	%.c %.dep Makefile
	$(CC) -x c $(CFLAGS) -c $< -o $@

clean:
	$(RM) jacklistenerd *.o *.dep

install:
	$(INSTALL) jacklistenerd /usr/sbin
	$(INSTALL) init.d/jacklistener /etc/init.d
	$(INSTALL) org.ude.jacklistener.conf /etc/dbus-1/system.d

uninstall:
	$(RM) /usr/sbin/jacklistenerd /etc/init.d/jacklistener /etc/dbus-1/system.d/org.ude.jacklistener.conf
