#include "io_device.h"
#include <cstdlib>
#include "cfutil.h"

io_device::io_device(struct udev_device* dev)
	: m_device(dev)
{
	m_properties = udev_device_get_properties_list_entry(m_device);
	m_sysattrs = udev_device_get_sysattr_list_entry(m_device);
}

io_device::~io_device()
{
	udev_device_unref(m_device);
}

const char* io_device::property(const char* name)
{
	struct udev_list_entry* p = udev_list_entry_get_by_name(m_properties, name);
	if (p != nullptr)
		return udev_list_entry_get_value(p);
	else
		return nullptr;
}

const char* io_device::sysattr(const char* name)
{
	struct udev_list_entry* p = udev_list_entry_get_by_name(m_sysattrs, name);
	if (p != nullptr)
		return udev_list_entry_get_value(p);
	else
		return nullptr;
}

CFStringRef io_device::sysattrStr(const char* name, CFAllocatorRef allocator)
{
	return strToCF(sysattr(name), allocator);
}

CFNumberRef io_device::sysattrNum(const char* name, CFAllocatorRef allocator)
{
	const char* value = sysattr(name);
	if (!value)
		return nullptr;
	
	char* end;
	long lvalue = strtol(value, &end, 16);

	if (end == value)
		return nullptr;
	
	return CFNumberCreate(allocator, kCFNumberLongType, &lvalue);
}

CFTypeRef io_device::retrieve(const property_mapping* mapping, CFAllocatorRef allocator)
{
	const char* value = nullptr;
	if (mapping->linuxProperty)
		value = property(mapping->linuxProperty);
	else if (mapping->linuxSysAttr)
		value = sysattr(mapping->linuxSysAttr);
	if (!value)
		return nullptr;
	
	if (mapping->dataType == property_mapping::String)
		return strToCF(value, allocator);
	else
	{
		int base = (mapping->dataType == property_mapping::Number10) ? 10 : 16;
		char* end;
		long lvalue = strtol(value, &end, base);

		if (end == value)
			return nullptr;

		return CFNumberCreate(allocator, kCFNumberLongType, &lvalue);
	}
}

void io_device::properties(CFMutableDictionaryRef dict, CFAllocatorRef allocator)
{
}

CFTypeRef io_device::property(CFStringRef name, CFAllocatorRef allocator)
{
	return nullptr;
}

const property_mapping* property_mapping::find(const property_mapping* map, size_t count, CFStringRef what)
{
	for (size_t i = 0; i < count; i++)
	{
		if (CFStringCompare(map[i].appleName, what, CFStringCompareFlags(0)) == 0)
			return map + i;
	}
	return nullptr;
}

