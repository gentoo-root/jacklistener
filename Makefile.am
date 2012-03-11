ACLOCAL_AMFLAGS = -I m4

sbin_PROGRAMS = jacklistenerd
jacklistenerd_SOURCES = main.c main.h dbus.c dbus.h event.c event.h
jacklistenerd_CFLAGS = $(AM_CFLAGS) $(DBUS_CFLAGS)
jacklistenerd_LDADD = $(DBUS_LIBS)

dbusconfigdir = $(sysconfdir)/dbus-1/system.d
dbusconfig_DATA = org.ude.jacklistener.conf

if CONFIG_OPENRC
openrcdir = $(sysconfdir)/init.d
openrc_DATA = init.d/openrc/jacklistener
endif

if CONFIG_ARCH
archrcdir = $(sysconfdir)/rc.d
archrc_DATA = init.d/arch/jacklistener
endif

if CONFIG_SYSTEMD
systemddir = $(libdir)/systemd/system
systemd_DATA = init.d/jacklistener.service
sbindirdir = $(sbindir)
sbindir_DATA = init.d/jacklistener-runscript
endif

if CONFIG_LSB
lsbrcdir = $(sysconfdir)/init.d
lsbrc_DATA = init.d/jacklistener
endif

AM_CFLAGS = -include $(top_builddir)/config.h
