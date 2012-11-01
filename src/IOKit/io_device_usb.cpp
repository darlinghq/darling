#include "io_device_usb.h"

// https://developer.apple.com/library/mac/#documentation/IOKit/Reference/USBSpec_header_reference/Reference/reference.html
static const property_mapping usb_properties[] = {
	{ CFSTR("Serial Number"), nullptr, "serial", property_mapping::String },
	{ CFSTR("USB Product Name"), nullptr, "product", property_mapping::String },
	{ CFSTR("USB Vendor Name"), nullptr, "manufacturer", property_mapping::String },
	{ CFSTR("VendorID"), nullptr, "idVendor", property_mapping::Number16 },
	{ CFSTR("ProductID"), nullptr, "idProduct", property_mapping::Number16 },
	{ CFSTR("bInterfaceClass"), nullptr, "bInterfaceClass", property_mapping::Number16 },
	{ CFSTR("bDeviceClass"), nullptr, "bDeviceClass", property_mapping::Number16 },
};


CFTypeRef io_device_usb::property(CFStringRef name, CFAllocatorRef allocator)
{
	const property_mapping* prop = property_mapping::find(usb_properties, sizeof(usb_properties)/sizeof(usb_properties[0]), name);

	if (!prop)
		return nullptr;
	else
		return retrieve(prop, allocator);
}

void io_device_usb::properties(CFMutableDictionaryRef dict, CFAllocatorRef allocator)
{
	// walk dict, call retrieve()

}
