.PHONY: first all clean install uninstall

SOURCES = main.c event.c dbus.c
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))
DEPS = $(patsubst %.c,%.dep,$(SOURCES))

SHELL = sh
CC = gcc
LD = gcc
INSTALL = install -v
RM = rm -f -v
RMDIR = rmdir -v

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
	$(INSTALL) -d $(DESTDIR)/usr/lib/jacklistener $(DESTDIR)/usr/sbin
	$(INSTALL) jacklistenerd $(DESTDIR)/usr/lib/jacklistener/
	$(INSTALL) jacklistener-runscript $(DESTDIR)/usr/sbin/jacklistenerd
	$(INSTALL) -d $(DESTDIR)/etc/init.d
	[ -x /sbin/runscript ] && $(INSTALL) init.d/jacklistener-openrc $(DESTDIR)/etc/init.d || $(INSTALL) init.d/jacklistener $(DESTDIR)/etc/init.d/
	$(INSTALL) -d $(DESTDIR)/lib/systemd/system
	$(INSTALL) init.d/jacklistener.service $(DESTDIR)/lib/systemd/system/
	$(INSTALL) -d $(DESTDIR)/etc/dbus-1/system.d
	$(INSTALL) org.ude.jacklistener.conf $(DESTDIR)/etc/dbus-1/system.d/

uninstall:
	$(RM) $(DESTDIR)/usr/lib/jacklistener/jacklistenerd $(DESTDIR)/usr/sbin/jacklistenerd $(DESTDIR)/etc/init.d/jacklistener $(DESTDIR)/lib/systemd/system/jacklistener.service $(DESTDIR)/etc/dbus-1/system.d/org.ude.jacklistener.conf
	$(RMDIR) $(DESTDIR)/usr/lib/jacklistener
