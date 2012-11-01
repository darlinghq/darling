#include "io_device_iterator.h"
#include "io_device.h"
#include <libudev.h>

io_device_iterator::io_device_iterator(udev_enumerate* uenum)
	: m_uenum(uenum)
{
	m_udev = udev_new();
	m_next = udev_enumerate_get_list_entry(uenum);
}

io_device_iterator::~io_device_iterator()
{
	udev_enumerate_unref(m_uenum);
	udev_unref(m_udev);
}

io_object_t io_device_iterator::next()
{
	if (!m_next)
		return nullptr;
	else
	{
		udev_device* dev = udev_device_new_from_syspath(m_udev, udev_list_entry_get_name(m_next));
		m_next = udev_list_entry_get_next(m_next);
		return new io_device(dev);
	}
}
