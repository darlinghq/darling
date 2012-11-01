#ifndef IOKIT_IODEVICE_H
#define IOKIT_IODEVICE_H
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>
#include <libudev.h>
#include "io_object.h"

struct property_mapping;

class io_device : public io_object
{
public:
	io_device(struct udev_device* device);
	virtual ~io_device();
	virtual void properties(CFMutableDictionaryRef dict, CFAllocatorRef allocator);
	virtual CFTypeRef property(CFStringRef name, CFAllocatorRef allocator);
protected:
	const char* property(const char* name);
	const char* sysattr(const char* name);
	CFStringRef sysattrStr(const char* name, CFAllocatorRef allocator);
	CFNumberRef sysattrNum(const char* name, CFAllocatorRef allocator);
	CFTypeRef retrieve(const property_mapping* mapping, CFAllocatorRef allocator);
protected:
	struct udev_device* m_device;
	struct udev_list_entry *m_properties, *m_sysattrs;
};

struct property_mapping
{
	CFStringRef appleName;
	const char* linuxProperty;
	const char* linuxSysAttr;

	enum DataType { String, Number10, Number16 };
	DataType dataType;

	static const property_mapping* find(const property_mapping* map, size_t count, CFStringRef what);
};

#endif

