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
#include <unistd.h>
#include <stdio.h>
#include <linux/input.h>

#include "main.h"
#include "dbus.h"

static void handle_event(int evdevfd);
static void handle_switch(__u16 code, __s32 value);

void parse_events(const fd_set *evdevfds)
{
	for (;;) {
		fd_set activefds = *evdevfds;
		if (select(FD_SETSIZE, &activefds, NULL, NULL, NULL) < 0) {
			perror("select");
			terminate(3);
		}

		int i;
		for (i=0; i<FD_SETSIZE; i++) {
			if (FD_ISSET(i, &activefds)) {
				handle_event(i);
			}
		}
	}
}

static void handle_event(int evdevfd)
{
	struct input_event event;
	int rbytes = read(evdevfd, &event, sizeof(event));

	if (rbytes == -1) {
		perror("read");
		terminate(4);
	} else
	if (rbytes == 0) {
		printf("EOF\n");
		terminate(4);
	} else
	if (rbytes != sizeof(event)) {
		printf("Invalid input\n");
		terminate(4);
	}

	switch (event.type) {
	case EV_SW:
		handle_switch(event.code, event.value);
		break;
	}
}

static void handle_switch(__u16 code, __s32 value)
{
	char *codename;
	switch (code) {
#ifdef SW_HEADPHONE_INSERT
	case SW_HEADPHONE_INSERT:
		codename = "headphone";
		break;
#else
#warning You have old kernel headers. Some features will be disabled. Please upgrade to Linux-3.2 or newer.
#endif

#ifdef SW_MICROPHONE_INSERT
	case SW_MICROPHONE_INSERT:
		codename = "microphone";
		break;
#else
#warning You have old kernel headers. Some features will be disabled. Please upgrade to Linux-3.2 or newer.
#endif

#ifdef SW_LINEOUT_INSERT
	case SW_LINEOUT_INSERT:
		codename = "lineout";
		break;
#else
#warning You have old kernel headers. Some features will be disabled. Please upgrade to Linux-3.2 or newer.
#endif

#ifdef SW_VIDEOOUT_INSERT
	case SW_VIDEOOUT_INSERT:
		codename = "videoout";
		break;
#else
#warning You have old kernel headers. Some features will be disabled. Please upgrade to Linux-3.2 or newer.
#endif

#ifdef SW_LINEIN_INSERT
	case SW_LINEIN_INSERT:
		codename = "linein";
		break;
#else
#warning You have old kernel headers. Some features will be disabled. Please upgrade to Linux-3.2 or newer.
#endif
	default:
		codename = "unknown";
		break;
	}

	char *state = value ? "inserted" : "removed";

	//printf("%s %s\n", codename, state);

	char path[50];
	snprintf(path, sizeof(path), "/jack/%s", codename);
	mp_dbus_send_signal(path, "org.ude.jacklistener", state, NULL, 0);
	mp_dbus_send_signal("/state", "org.ude.jacklistener", "statechanged", codename, value);
}
