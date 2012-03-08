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

#include <dbus/dbus.h>
#include <stdio.h>

static DBusError error;
static DBusConnection *connection;

int mp_dbus_init()
{
	dbus_error_init(&error);

	int res;

	// Connect
	connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	if (dbus_error_is_set(&error)) {
		fprintf(stderr, "DBus: dbus_bus_get: %s\n", error.message);
		dbus_error_free(&error);
		return 0;
	}
	if (connection == NULL) {
		fprintf(stderr, "DBus: connection failed\n");
		dbus_error_free(&error);
		return 0;
	}

	// Request name
	res = dbus_bus_request_name(connection, "org.ude.jacklistener", DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
	if (dbus_error_is_set(&error)) {
		fprintf(stderr, "DBus: dbus_bus_request_name: %s\n", error.message);
		dbus_error_free(&error);
		return 0;
	}
	if (res != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		fprintf(stderr, "DBus: name request failed\n");
		dbus_connection_close(connection);
		dbus_error_free(&error);
		return 0;
	}

	return 1;
}

void mp_dbus_fini()
{
	dbus_connection_close(connection);
}

int mp_dbus_send_signal(const char *path, const char *interface, const char *name, const char *str, int state)
{
	dbus_uint32_t serial = 0;

	// Create signal
	DBusMessage *msg = dbus_message_new_signal(path, interface, name);
	if (msg == NULL) {
		fprintf(stderr, "DBus: error creating signal\n");
		return 0;
	}

	// Append arguments
	if (str != NULL) {
		DBusMessageIter args;
		dbus_message_iter_init_append(msg, &args);
		if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &str)) {
			fprintf(stderr, "DBus: error building signal\n");
			return 0;
		}
		if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &state)) {
			fprintf(stderr, "DBus: error building signal\n");
			return 0;
		}
	}

	// Send signal
	if (!dbus_connection_send(connection, msg, &serial)) {
		fprintf(stderr, "DBus: error sending signal\n");
		return 0;
	}

	dbus_connection_flush(connection);

	// Delete signal
	dbus_message_unref(msg);

	return 1;
}
