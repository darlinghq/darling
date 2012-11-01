#include "io_device_net.h"
#include "cfutil.h"
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_arp.h>

/*
#define kIOInterfaceUnit		"IOInterfaceUnit"
#define kIOInterfaceType		"IOInterfaceType"
#define kIOMACAddress			"IOMACAddress"
#define kIOPrimaryInterface		"IOPrimaryInterface"
*/

static CFTypeRef ConvertInterfaceType(io_device* d, CFAllocatorRef allocator);
static CFTypeRef ConvertMACAddress(io_device* d, CFAllocatorRef allocator);

static const property_mapping iface_properties[] = {
	property_mapping{ CFSTR("IOMediaAddressLength"), nullptr, "addr_len", property_mapping::Number10 },
	property_mapping{ CFSTR("IOInterfaceType"), ConvertInterfaceType },
	property_mapping{ CFSTR("IOMaxTransferUnit"), nullptr, "mtu", property_mapping::Number10 },
	property_mapping{ CFSTR("IOBuiltin"), [](io_device* d, CFAllocatorRef) -> CFTypeRef { return kCFBooleanFalse; } },
	property_mapping{ CFSTR("IOPrimaryInterface"), [](io_device* d, CFAllocatorRef) -> CFTypeRef { return (strcmp(d->property("INTERFACE"), "eth0") == 0) ? kCFBooleanTrue : kCFBooleanFalse; } },
};

CFTypeRef io_device_net::property(CFStringRef name, CFAllocatorRef allocator)
{
	CFTypeRef v = retrieve(iface_properties, sizeof(iface_properties) / sizeof(iface_properties[0]), name, allocator);
	if (v)
		return v;
	else
		return io_device::property(name, allocator);
}

void io_device_net::properties(CFMutableDictionaryRef dict, CFAllocatorRef allocator)
{
	io_device::properties(dict, allocator);
	retrieveAll(dict, iface_properties, sizeof(iface_properties) / sizeof(iface_properties[0]), allocator);
}

static CFTypeRef ConvertInterfaceType(io_device* d, CFAllocatorRef allocator)
{
	int type = atoi(d->sysattr("type"));
	int mtype = 0;
	
	// This is not complete, but could suffice
	switch (type)
	{
	case ARPHRD_ETHER:
	case ARPHRD_IEEE80211: // linux/include/linux/if_arp.h
		mtype = 6; // xnu-792/bsd/net/if_types.h
		break;
	case ARPHRD_LOOPBACK:
		mtype = 0x18;
		break;
	case ARPHRD_PPP:
		mtype = 0x17;
		break;
	case ARPHRD_ATM:
		mtype = 0x25;
		break;
	case ARPHRD_IEEE1394:
		mtype = 0x90;
		break;
	case ARPHRD_SLIP:
		mtype = 0x1c;
		break;
	}
	
	return intToCF(mtype, allocator);
}

io_device* io_device_net::parent()
{
	return new io_device_net_ctrl(udev_device_ref(m_device));
}

static const property_mapping ctrl_properties[] = {
	property_mapping{ CFSTR("IOLinkSpeed"), nullptr, "speed", property_mapping::Number10 },
	property_mapping{ CFSTR("IOLinkStatus"), [](io_device* d, CFAllocatorRef a) -> CFTypeRef { int act = atoi(d->sysattr("carrier")); return intToCF(act ? 3 : 1, a); } }, // if_media.h
	property_mapping{ CFSTR("IOMACAddress"), ConvertMACAddress },
};

CFTypeRef io_device_net_ctrl::property(CFStringRef name, CFAllocatorRef allocator)
{
	return io_device::property(name, allocator);
}

void io_device_net_ctrl::properties(CFMutableDictionaryRef dict, CFAllocatorRef allocator)
{
	io_device::properties(dict, allocator);
}

CFTypeRef ConvertMACAddress(io_device* d, CFAllocatorRef allocator)
{
	int len = atoi(d->sysattr("addr_len"));
	const char* addr = d->sysattr("address");
	
	if (!addr)
		return nullptr;
	
	UInt8* bytes = new UInt8[len];
	CFTypeRef rv;
	
	for (int i = 0; i < len; i++)
	{
		long byte = strtol(addr + i*3, nullptr, 16);
		bytes[i] = UInt8(byte);
	}
	
	rv = CFDataCreate(allocator, bytes, len);
	
	delete [] bytes;
	return rv;
}

