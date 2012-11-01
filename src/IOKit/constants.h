#ifndef IOKIT_CONSTANTS_H
#define IOKIT_CONSTANTS_H

#define kIOMasterPortDefault NULL

#define kIOProviderClassKey		"IOProviderClass"
#define kIOBSDNameKey			"BSD Name"
#define kIONameMatchKey			"IONameMatch"
#define kIOPropertyMatchKey		"IOPropertyMatch"
#define kIOPathMatchKey			"IOPathMatch"
#define kIOLocationMatchKey		"IOLocationMatch"
#define kIOParentMatchKey		"IOParentMatch"
#define kIOResourceMatchKey		"IOResourceMatch"
#define kIOMatchedServiceCountKey	"IOMatchedServiceCountMatch"
#define kIONameMatchedKey		"IONameMatched"
#define kIOMatchCategoryKey		"IOMatchCategory"
#define kIODefaultMatchCategoryKey	"IODefaultMatchCategory"

// ethernet
#define kIOEthernetAddressSize 6
#define kIOEthernetInterfaceClass "IOEthernetInterface"

// serial
#define kIOSerialBSDServiceValue	"IOSerialBSDClient"
#define kIOSerialBSDAllTypes		"IOSerialStream"
#define kIOSerialBSDModemType		"IOModemSerialStream"
#define kIOSerialBSDRS232Type		"IORS232SerialStream"

#define kIOTTYDeviceKey			"IOTTYDevice"
#define kIOTTYBaseNameKey		"IOTTYBaseName"
#define kIOTTYSuffixKey			"IOTTYSuffix"
#define kIOCalloutDeviceKey		"IOCalloutDevice"
#define kIODialinDeviceKey		"IODialinDevice"

// USB
#define kIOUSBDeviceClassName		"IOUSBDevice"
#define kIOUSBInterfaceClassName	"IOUSBInterface"
#define kIOHIDDeviceKey				"IOHIDDevice"

#endif

