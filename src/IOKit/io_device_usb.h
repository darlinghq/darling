#ifndef IOKIT_IODEVICEUSB_H
#define IOKIT_IODEVICEUSB_H
#include "io_device.h"

class io_device_usb : public io_device
{
public:
	// error: inheriting constructors are not supported
	// using io_device::io_device;
	
	io_device_usb(struct udev_device* device) : io_device(device) {}
	virtual CFTypeRef property(CFStringRef name, CFAllocatorRef allocator) override;
	virtual void properties(CFMutableDictionaryRef dict, CFAllocatorRef allocator) override;
};

#endif
