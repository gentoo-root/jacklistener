#include "watchdevs.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>

#include <uthash.h>

#ifdef ENABLE_UDEV
#include <libudev.h>
#endif

static void hash_element_free(struct hash_element *elem)
{
	free(elem->devnode);
	if (elem->fd >= 0) {
		close(elem->fd);
	}
	free(elem);
}

static struct hash_element *hash_element_from_devnode(const char *devnode)
{
	struct hash_element *elem = malloc(sizeof(*elem));
	elem->devnode = strdup(devnode);
	elem->fd = open(devnode, O_RDONLY);
	if (elem->fd < 0) {
		fprintf(stderr, "Error opening %s: %s\n", devnode, strerror(errno));
		goto return_null;
	}
	char device_name[1024];
	if (ioctl(elem->fd, EVIOCGNAME(sizeof(device_name)), device_name) < 0) {
		perror("ioctl");
		fprintf(stderr, "Not event device: '%s'\n", devnode);
		goto return_null;
	}
	if (!is_quiet) {
		fprintf(stderr, "Opened device '%s'\n", device_name);
	}

	goto end;
return_null:
	hash_element_free(elem);
	elem = NULL;
end:
	return elem;
}

static void listen_device_if_not_present(const char *devnode)
{
	struct hash_element *same_elem = NULL;
	HASH_FIND_STR(fdhash, devnode, same_elem);
	if (same_elem) {
		return;
	}
	struct hash_element *elem = hash_element_from_devnode(devnode);
	if (!elem) {
		return;
	}
	HASH_ADD_KEYPTR(hh, fdhash, elem->devnode, strlen(elem->devnode), elem);
	FD_SET(elem->fd, &evdevfds);
}

#ifdef ENABLE_UDEV

void unlisten_device_by_hash_element(struct hash_element *elem)
{
	close(elem->fd);
	FD_CLR(elem->fd, &evdevfds);
	HASH_DEL(fdhash, elem);
	hash_element_free(elem);
}

static void unlisten_device_by_devnode(const char *devnode)
{
	struct hash_element *elem = NULL;
	HASH_FIND_STR(fdhash, devnode, elem);
	if (!elem) {
		return;
	}
	unlisten_device_by_hash_element(elem);
}

void unlisten_all_devices(void)
{
	if (!fdhash) {
		return;
	}
	struct hash_element *elem, *tmp;
	HASH_ITER(hh, fdhash, elem, tmp) {
		unlisten_device_by_hash_element(elem);
	}
}

static int is_alsa_device(struct udev_device *dev)
{
	const char *phys = udev_device_get_sysattr_value(dev, "device/phys");
	return phys && strcmp(phys, "ALSA") == 0;
}

static void process_new_device(struct udev_device *dev)
{
	if (!is_alsa_device(dev)) {
		return;
	}
	const char *devnode = udev_device_get_devnode(dev);
	if (!devnode) {
		fprintf(stderr, "Device has no devnode\n");
		return;
	}

	listen_device_if_not_present(devnode);
}

static void process_removed_device(struct udev_device *dev)
{
	if (!is_alsa_device(dev)) {
		return;
	}
	const char *devnode = udev_device_get_devnode(dev);
	if (!devnode) {
		return;
	}
	unlisten_device_by_devnode(devnode);
}

void init_devices_udev(void)
{
	struct udev_enumerate *enumerate = udev_enumerate_new(udev);
	if (!enumerate) {
		fprintf(stderr, "%s: Cannot create enumerate context\n", __PRETTY_FUNCTION__);
		return;
	}
	udev_enumerate_add_match_subsystem(enumerate, "input");
	udev_enumerate_scan_devices(enumerate);
	struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
	struct udev_list_entry *device_entry;
	udev_list_entry_foreach(device_entry, devices) {
		const char *path = udev_list_entry_get_name(device_entry);
		struct udev_device *dev = udev_device_new_from_syspath(udev, path);
		process_new_device(dev);
		udev_device_unref(dev);
	}

	udev_enumerate_unref(enumerate);
}

struct udev_monitor *init_monitor(void)
{
	struct udev_monitor *mon = udev_monitor_new_from_netlink(udev, "udev");
	if (!mon) {
		fprintf(stderr, "Cannot create monitor\n");
		return NULL;
	}
	udev_monitor_filter_add_match_subsystem_devtype(mon, "input", NULL);
	udev_monitor_enable_receiving(mon);
	return mon;
}

void handle_monitor_event(void)
{
	struct udev_device *dev = udev_monitor_receive_device(mon);
	if (!dev) {
		return;
	}
	const char *action = udev_device_get_action(dev);
	if (!action) {
		goto free_resources;
	}
	if (strcmp(action, "add") == 0) {
		process_new_device(dev);
	} else if (strcmp(action, "remove") == 0) {
		process_removed_device(dev);
	}
free_resources:
	udev_device_unref(dev);
}

#endif // ENABLE_UDEV

void init_devices_list(int ndevs, char *devnodes[])
{
	for (int i = 0; i < ndevs; ++i) {
		listen_device_if_not_present(devnodes[i]);
	}
}
