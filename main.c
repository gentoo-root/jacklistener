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

#include "main.h"
#include "event.h"
#include "dbus.h"

static fd_set evdevfds;
FILE *pidfile;

static void usage();
static void openfiles(int count, char *names[]);
static void open_pidfile(char *path);

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		usage(argv[0]);
	}

	// Parse options
	int is_daemon = 0;
	int is_pidfile = 0;
	const struct option options[] = {
		{ "daemon", no_argument, NULL, 'd' },
		{ "pidfile", required_argument, NULL, 'p' },
		{ NULL, 0, NULL, 0 }
	};
	int optparse;
	//opterr = 0;
	while ((optparse = getopt_long(argc, argv, "dp:", options, NULL)) != -1) {
		switch (optparse) {
		case 'd':
			is_daemon = 1;
			break;
		case 'p':
			is_pidfile = 1;
			open_pidfile(optarg);
			break;
		case ':':
		case '?':
		default:
			fprintf(stderr, "Invalid options\n");
			usage(argv[0]);
			break;
		}
	}

	if (optind >= argc) {
		usage(argv[0]);
	}

	openfiles(argc-optind, argv+optind);
	if (!mp_dbus_init()) {
		fprintf(stderr, "Unable to initialize D-Bus\n");
		exit(13);
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
			fprintf(stderr, "Daemonized successfully\n");
			exit(0);
		}
	}

	setuid(getpwnam("daemon")->pw_uid);

	if (is_pidfile) {
		fprintf(pidfile, "%d", getpid());
		fclose(pidfile);
	}

	parse_events(&evdevfds);
	terminate(0);

	return 0;
}

static void usage(char *argv0)
{
	fprintf(stderr, "Usage: %s [-d|--daemon] <event device files>\n", argv0);
	exit(254);
}

static void openfiles(int count, char *names[])
{
	FD_ZERO(&evdevfds);

	int i;
	for (i=0; i<count; i++) {
		int evdevfd;
		if ((evdevfd = open(names[i], O_RDONLY)) < 0) {
			perror("open");
		}
	
		char device_name[1024];
		if ((ioctl(evdevfd, EVIOCGNAME(sizeof(device_name)), device_name)) < 0) {
			perror("ioctl");
			printf("Not event device: \"%s\"\n", names[i]);
			close(evdevfd);
		} else {
			printf("Opened device \"%s\"\n", device_name);
			FD_SET(evdevfd, &evdevfds);
		}
	}
}

void terminate(int exitcode)
{
	mp_dbus_fini();

	int i;
	for (i=0; i<FD_SETSIZE; i++) {
		if (FD_ISSET(i, &evdevfds)) {
			close(i);
		}
	}

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
