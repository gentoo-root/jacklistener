/*
 * This file is part of jacklistener - jack state monitor
 * Copyright (C) 2012 Maxim A. Mikityanskiy - <maxtram95@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <getopt.h>
#include <pwd.h>
#include <string.h>
#include <errno.h>

#ifdef ENABLE_UDEV
#include <libudev.h>
#endif

#include "main.h"
#include "event.h"
#include "dbus.h"
#include "watchdevs.h"

static FILE *pidfile;
int is_quiet = 0;
static int is_udev = 0;

static int has_udev =
#ifdef ENABLE_UDEV
	1
#else
	0
#endif
	;

fd_set evdevfds;
struct hash_element *fdhash = NULL;

static void usage(const char *argv0);
static void open_pidfile(char *path);

static void parse_events(void);

#ifdef ENABLE_UDEV
struct udev *udev = NULL;
struct udev_monitor *mon = NULL;
#endif

enum {
	ARG_HAS_UDEV = 1000,
};

int main(int argc, char *argv[])
{
	// Parse options
	int is_daemon = 0;
	int is_pidfile = 0;
	int is_query_has_udev = 0;
	const struct option options[] = {
		{ "quiet", no_argument, NULL, 'q' },
		{ "daemon", no_argument, NULL, 'd' },
		{ "pidfile", required_argument, NULL, 'p' },
#ifdef ENABLE_UDEV
		{ "udev", no_argument, NULL, 'u'},
#endif
		{ "has-udev", no_argument, NULL, ARG_HAS_UDEV },
		{ NULL, 0, NULL, 0 }
	};
	int optparse;
	const char *optchars = "qdp:"
#ifdef ENABLE_UDEV
	"u"
#endif
	;

	//opterr = 0;
	while ((optparse = getopt_long(argc, argv, optchars, options, NULL)) != -1) {
		switch (optparse) {
		case 'q':
			is_quiet = 1;
			break;
		case 'd':
			is_daemon = 1;
			break;
		case 'p':
			is_pidfile = 1;
			open_pidfile(optarg);
			break;
#ifdef ENABLE_UDEV
		case 'u':
			is_udev = 1;
			break;
#endif
		case ARG_HAS_UDEV:
			is_query_has_udev = 1;
			break;
		case ':':
		case '?':
		default:
			fprintf(stderr, "Invalid options\n");
			usage(argv[0]);
			break;
		}
	}

	if (is_query_has_udev) {
		exit(!has_udev);
	}

	if (!is_udev && optind >= argc) {
		usage(argv[0]);
	}

	if (is_udev && optind != argc) {
		usage(argv[0]);
	}

	if (!mp_dbus_init()) {
		fprintf(stderr, "Unable to initialize D-Bus\n");
		exit(13);
	}

	FD_ZERO(&evdevfds);

#ifdef ENABLE_UDEV
	if (is_udev) {
		udev = udev_new();
		if (!udev) {
			fprintf(stderr, "%s: Cannot create udev context\n", __PRETTY_FUNCTION__);
			terminate(1);
		}
	}
#endif
	if (!is_udev) {
		init_devices_list(argc - optind, argv + optind);
	}

	if (is_daemon) {
		switch (fork()) {
		case -1:
			terminate(63);
		case 0:
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			if (chdir("/")) ;
			if (setsid() == -1) {
				terminate(63);
			}
			break;
		default:
			if (!is_quiet) {
				fprintf(stderr, "Daemonized successfully\n");
			}
			exit(0);
		}
	}

	if (is_pidfile) {
		fprintf(pidfile, "%d", getpid());
		fclose(pidfile);
	}

	#ifdef ENABLE_UDEV
	if (is_udev) {
		mon = init_monitor();
		init_devices_udev();
	}
	#endif
	if (!is_udev) {
		setuid(getpwnam("daemon")->pw_uid);
	}

	parse_events();

	terminate(0);

	return 0;
}

static void parse_events(void)
{
#ifdef ENABLE_UDEV
	int mon_fd = -1;
	if (is_udev) {
		mon_fd = udev_monitor_get_fd(mon);
	}
#endif
	while (1) {
		fd_set activefds = evdevfds;
#ifdef ENABLE_UDEV
		if (is_udev) {
			FD_SET(mon_fd, &activefds);
		}
#endif
		if (select(FD_SETSIZE, &activefds, NULL, NULL, NULL) == -1) {
			fprintf(stderr, "Error when polling events in select(): %s\n", strerror(errno));
			terminate(3);
		}
		for (struct hash_element *elem = fdhash; elem != NULL; elem = elem->hh.next) {
			if (FD_ISSET(elem->fd, &activefds)) {
				if (!handle_device_event(elem->fd)) {
					unlisten_device_by_hash_element(elem);
				}
			}
		}
#ifdef ENABLE_UDEV
		if (is_udev && FD_ISSET(mon_fd, &activefds)) {
			handle_monitor_event();
		}
#endif
	}
}

static void usage(const char *argv0)
{
	const char *udev_args_desc = ""
#ifdef ENABLE_UDEV
	"[-u|--udev]|"
#endif
	;
	fprintf(stderr, "Usage: %s --has-udev | ([-q|--quiet] [-d|--daemon] [-p|--pidfile <pidfile>] %s<event device files>)\n", argv0, udev_args_desc);
	exit(254);
}

void terminate(int exitcode)
{
	mp_dbus_fini();
#ifdef ENABLE_UDEV
	if (is_udev) {
		unlisten_all_devices();
		if (mon) {
			udev_monitor_unref(mon);
		}
		if (udev) {
			udev_unref(udev);
		}
	}
#endif
	exit(exitcode);
}

static void open_pidfile(char *path)
{
	if ((pidfile = fopen(path, "w")) == NULL) {
		perror("fopen");
		fprintf(stderr, "Cannot open pidfile \"%s\"\n", path);
		exit(31);
	}
}
