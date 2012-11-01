#ifndef IOKIT_IODEVICENET_H
#define IOKIT_IODEVICENET_H
#include "io_device.h"

// IONetworkInterface
class io_device_net : public io_device
{
public:
	// error: inheriting constructors are not supported
	// using io_device::io_device;
	
	io_device_net(struct udev_device* device) : io_device(device) {}
	virtual CFTypeRef property(CFStringRef name, CFAllocatorRef allocator) override;
	virtual void properties(CFMutableDictionaryRef dict, CFAllocatorRef allocator) override;
	virtual io_device* parent() override;
};

// IONetworkController
class io_device_net_ctrl : public io_device
{
public:
	io_device_net_ctrl(struct udev_device* device) : io_device(device) {}
	virtual CFTypeRef property(CFStringRef name, CFAllocatorRef allocator) override;
	virtual void properties(CFMutableDictionaryRef dict, CFAllocatorRef allocator) override;
};

#endif
