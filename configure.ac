AC_COPYRIGHT([Copyright (c) 2012 Maxim A. Mikityanskiy <maxtram95@gmail.com>])
AC_INIT([jacklistener], [0.1], [https://github.com/gentoo-root/jacklistener/issues/])
AM_INIT_AUTOMAKE([AC_PACKAGE_TARNAME()], [AC_PACKAGE_VERSION()])
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE()

LT_INIT()
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

AC_HEADER_STDC()
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])

#XDT_CHECK_PACKAGE([DBUS], [dbus-1], [1.4])
PKG_CHECK_MODULES(DBUS, [ dbus-1 >= 1.3.2 ])

AC_ARG_ENABLE(openrc, AS_HELP_STRING([--enable-openrc], [Enable OpenRC initscript]),
[case "${enableval}" in
	yes) openrc=yes;;
	no) openrc=no;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-openrc) ;;
esac], [openrc=no])
AM_CONDITIONAL(CONFIG_OPENRC, test "$openrc" = yes)

AC_ARG_ENABLE(arch, AS_HELP_STRING([--enable-arch], [Enable Arch initscript]),
[case "${enableval}" in
	yes) arch=yes;;
	no) arch=no;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-arch) ;;
esac], [arch=no])
AM_CONDITIONAL(CONFIG_ARCH, test "$arch" = yes)

AC_ARG_ENABLE(systemd, AS_HELP_STRING([--enable-systemd], [Enable systemd unit]),
[case "${enableval}" in
	yes) systemd=yes;;
	no) systemd=no;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-systemd) ;;
esac], [systemd=no])
AM_CONDITIONAL(CONFIG_SYSTEMD, test "$systemd" = yes)

AC_ARG_ENABLE(lsb, AS_HELP_STRING([--enable-lsb], [Enable LSB initscript]),
[case "${enableval}" in
	yes) lsb=yes;;
	no) lsb=no;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-lsb) ;;
esac], [lsb=no])
AM_CONDITIONAL(CONFIG_LSB, test "$lsb" = yes)

test "$lsb" = yes && test "$openrc" = yes && AC_MSG_ERROR(incompatible options. Choose one of --enable-openrc and --enable-lsb)

AC_OUTPUT([
Makefile
])
