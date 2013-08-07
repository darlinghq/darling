#include "io_device.h"
#include <cstdlib>
#include <cstring>
#include "io_device_net.h"
#include "cfutil.h"
#include <util/debug.h>

static CFTypeRef ListDevSymlinks(io_device* d, CFAllocatorRef a);

static const property_mapping generic_properties[] = {
	property_mapping{ CFSTR("IOVendor"), [](io_device* d, CFAllocatorRef a) -> CFTypeRef { if (const char* name = d->property("ID_VENDOR")) return strToCF(name, a); else return strToCF(d->property("ID_VENDOR_FROM_DATABASE"), a); } },
	property_mapping{ CFSTR("IOModel"), [](io_device* d, CFAllocatorRef a) -> CFTypeRef { if (const char* name = d->property("ID_MODEL")) return strToCF(name, a); else return strToCF(d->property("ID_MODEL_FROM_DATABASE"), a); } },
	property_mapping{ CFSTR("BSD Name"), [](io_device* d, CFAllocatorRef a) -> CFTypeRef { if (const char* name = d->sysname()) return strToCF(name, a); else return strToCF(d->property("INTERFACE"), a);  } }, // name or property INTERFACE
	property_mapping{ CFSTR("BSD Names"), ListDevSymlinks },
	property_mapping{ CFSTR("BSD Major"), [](io_device* d, CFAllocatorRef a) -> CFTypeRef { return intToCF(d->property("MAJOR"), a); } },
	property_mapping{ CFSTR("BSD Minor"), [](io_device* d, CFAllocatorRef a) -> CFTypeRef { return intToCF(d->property("MINOR"), a); } },
};

io_device::io_device(struct udev_device* dev)
	: m_device(dev)
{
}

io_device::~io_device()
{
	udev_device_unref(m_device);
}

io_device* io_device::create(udev_device* dev)
{
	const char* subsystem = udev_device_get_subsystem(dev);
	
	LOG << "Creating an io_device for subsystem " << subsystem << std::endl;
	
	if (strcmp(subsystem, "net") == 0)
		return new io_device_net(dev);
	else
		return new io_device(dev); // generic device
}

bool io_device::operator==(const io_object& that)
{
	const io_device* dthat = dynamic_cast<const io_device*>(&that);
	if (!dthat)
		return false;
	
	return strcmp(udev_device_get_syspath(m_device), udev_device_get_syspath(dthat->m_device)) == 0;
}

const char* io_device::property(const char* name)
{
	return udev_device_get_property_value(m_device, name);
}

const char* io_device::sysattr(const char* name)
{
	return udev_device_get_sysattr_value(m_device, name);
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
	if (mapping->evaluator)
		return mapping->evaluator(this, allocator);
	else
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
}

void io_device::retrieveAll(CFMutableDictionaryRef dict, const property_mapping* mapping, size_t count, CFAllocatorRef allocator)
{
	for (size_t i = 0; i < count; i++)
	{
		CFTypeRef val = retrieve(mapping + i, allocator);
		CFDictionarySetValue(dict, mapping[i].appleName, val);
	}
}

CFTypeRef io_device::retrieve(const property_mapping* mapping, size_t count, CFStringRef name, CFAllocatorRef allocator)
{
	for (size_t i = 0; i < count; i++)
	{
		if (CFStringCompare(name, mapping[i].appleName, CFStringCompareFlags(0)) == 0)
			return retrieve(mapping + i, allocator);
	}
	return nullptr;
}

io_device* io_device::parent()
{
	return nullptr; // not implemented yet
}

const char* io_device::sysname()
{
	return udev_device_get_sysname(m_device);
}

void io_device::properties(CFMutableDictionaryRef dict, CFAllocatorRef allocator)
{
	retrieveAll(dict, generic_properties, sizeof(generic_properties) / sizeof(generic_properties[0]), allocator);
}

CFTypeRef io_device::property(CFStringRef name, CFAllocatorRef allocator)
{
	return retrieve(generic_properties, sizeof(generic_properties) / sizeof(generic_properties[0]), name, allocator);
}

CFTypeRef ListDevSymlinks(io_device* d, CFAllocatorRef a)
{
	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(a, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	struct udev_list_entry* e = udev_device_get_devlinks_list_entry(d->device());
	
	while (e != nullptr)
	{
		CFDictionarySetValue(dict, strToCF(udev_list_entry_get_name(e)), strToCF(udev_list_entry_get_value(e)));
		e = udev_list_entry_get_next(e);
	}
	
	return dict;
}
