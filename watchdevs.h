#pragma once

#include <sys/select.h>
#include <uthash.h>

struct hash_element
{
	char *devnode;
	int fd;
	UT_hash_handle hh;
};

void unlisten_device_by_hash_element(struct hash_element *elem);

#ifdef ENABLE_UDEV
struct udev_monitor;

void init_devices_udev(void);
struct udev_monitor *init_monitor(void);
void handle_monitor_event(void);
void unlisten_all_devices(void);
#endif

void init_devices_list(int ndevs, char *devnodes[]);
