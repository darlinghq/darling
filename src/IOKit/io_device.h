#ifndef IOKIT_IODEVICE_H
#define IOKIT_IODEVICE_H
extern "C" {
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>
}
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
	virtual io_device* parent();
	virtual bool operator==(const io_object& that) override;
	
	const char* sysname();
	const char* property(const char* name);
	const char* sysattr(const char* name);
	CFStringRef sysattrStr(const char* name, CFAllocatorRef allocator);
	CFNumberRef sysattrNum(const char* name, CFAllocatorRef allocator);
	inline struct udev_device* device() { return m_device; }
protected:
	CFTypeRef retrieve(const property_mapping* mapping, CFAllocatorRef allocator);
	CFTypeRef retrieve(const property_mapping* mapping, size_t count, CFStringRef name, CFAllocatorRef allocator);
	void retrieveAll(CFMutableDictionaryRef dict, const property_mapping* mapping, size_t count, CFAllocatorRef allocator);
protected:
	struct udev_device* m_device;
	struct udev_list_entry *m_properties, *m_sysattrs;
};

struct property_mapping
{
	enum DataType { String, Number10, Number16 };
	typedef CFTypeRef (*EvaluatorType)(io_device*,CFAllocatorRef);
	
	property_mapping(CFStringRef appleName, const char* linuxProperty, const char* linuxSysAttr, DataType dataType)
	: appleName(appleName), linuxProperty(linuxProperty), linuxSysAttr(linuxSysAttr), dataType(dataType), evaluator(0)
	{
	}
	
	property_mapping(CFStringRef appleName, EvaluatorType ev)
	: appleName(appleName), evaluator(ev)
	{
	}
	
	CFStringRef appleName;
	const char* linuxProperty;
	const char* linuxSysAttr;

	DataType dataType;
	EvaluatorType evaluator;
};

#endif
