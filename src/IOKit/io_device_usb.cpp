#include "io_device_usb.h"

// https://developer.apple.com/library/mac/#documentation/IOKit/Reference/USBSpec_header_reference/Reference/reference.html
static const property_mapping usb_properties[] = {
	property_mapping{ CFSTR("Serial Number"), nullptr, "serial", property_mapping::String },
	property_mapping{ CFSTR("USB Product Name"), nullptr, "product", property_mapping::String },
	property_mapping{ CFSTR("USB Vendor Name"), nullptr, "manufacturer", property_mapping::String },
	property_mapping{ CFSTR("VendorID"), nullptr, "idVendor", property_mapping::Number16 },
	property_mapping{ CFSTR("ProductID"), nullptr, "idProduct", property_mapping::Number16 },
	property_mapping{ CFSTR("bInterfaceClass"), nullptr, "bInterfaceClass", property_mapping::Number16 },
	property_mapping{ CFSTR("bDeviceClass"), nullptr, "bDeviceClass", property_mapping::Number16 },
};


CFTypeRef io_device_usb::property(CFStringRef name, CFAllocatorRef allocator)
{
	CFTypeRef v = retrieve(usb_properties, sizeof(usb_properties) / sizeof(usb_properties[0]), name, allocator);
	if (v)
		return v;
	else
		return io_device::property(name, allocator);
}

void io_device_usb::properties(CFMutableDictionaryRef dict, CFAllocatorRef allocator)
{
	io_device::properties(dict, allocator);
	retrieveAll(dict, usb_properties, sizeof(usb_properties) / sizeof(usb_properties[0]), allocator);
}
