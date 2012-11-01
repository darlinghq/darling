#ifndef IOKIT_IODEVICEITERATOR_H
#define IOKIT_IODEVICEITERATOR_H
#include "io_iterator.h"

class io_device_iterator : public io_iterator
{
public:
	io_device_iterator(struct udev_enumerate* uenum);
	virtual ~io_device_iterator();
	virtual io_object_t next() override;
private:
	struct udev* m_udev;
	struct udev_enumerate* m_uenum;
	struct udev_list_entry* m_next;
};

typedef io_device_iterator* io_device_iterator_t;

#endif
