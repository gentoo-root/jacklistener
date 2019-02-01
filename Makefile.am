ACLOCAL_AMFLAGS = -I m4

edit = sed -e 's|@sbindir[@]|$(sbindir)|g'

sbin_PROGRAMS = jacklistenerd
jacklistenerd_SOURCES = main.c main.h dbus.c dbus.h event.c event.h watchdevs.c watchdevs.h
jacklistenerd_CFLAGS = $(AM_CFLAGS) $(DBUS_CFLAGS)
jacklistenerd_LDADD = $(DBUS_LIBS)

dbusconfigdir = $(sysconfdir)/dbus-1/system.d
dbusconfig_DATA = org.ude.jacklistener.conf

sbindirdir = $(sbindir)
sbindir_DATA = init.d/jacklistener-runscript
init.d/jacklistener-runscript: $(srcdir)/init.d/jacklistener-runscript.in Makefile
	$(edit) $< > $@

if CONFIG_OPENRC
openrcdir = $(sysconfdir)/init.d
openrc_DATA = init.d/openrc/jacklistener
init.d/openrc/jacklistener: $(srcdir)/init.d/openrc/jacklistener.in Makefile
	$(edit) $< > $@
endif

if CONFIG_ARCH
archrcdir = $(sysconfdir)/rc.d
archrc_DATA = init.d/arch/jacklistenerd
endif

if CONFIG_SYSTEMD
systemddir = $(libdir)/systemd/system
systemd_DATA = init.d/jacklistener.service
init.d/jacklistener.service: $(srcdir)/init.d/jacklistener.service.in Makefile
	$(edit) $< > $@
endif

if CONFIG_LSB
lsbrcdir = $(sysconfdir)/init.d
lsbrc_DATA = init.d/jacklistener
endif

if CONFIG_UDEV
jacklistenerd_CFLAGS += $(UDEV_CFLAGS) -DENABLE_UDEV
jacklistenerd_LDADD += $(UDEV_LIBS)
endif

AM_CFLAGS = -include $(top_builddir)/config.h

install-data-hook:
if CONFIG_OPENRC
	chmod +x $(DESTDIR)$(openrcdir)/jacklistener
endif
if CONFIG_ARCH
	chmod +x $(DESTDIR)$(archrcdir)/jacklistenerd
endif
if CONFIG_LSB
	chmod +x $(DESTDIR)$(lsbrcdir)/jacklistener
endif
	chmod +x $(DESTDIR)$(sbindir)/jacklistener-runscript
