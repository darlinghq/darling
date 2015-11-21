#include <CoreFoundation/CFBase.h>

// A real implementation would use VA API or VDPAU

#define kVDADecoderHardwareNotSupportedErr -12470

OSStatus VDADecoderCreate()
{
	return kVDADecoderHardwareNotSupportedErr;
}

OSStatus VDADecoderDestroy()
{
	return kVDADecoderHardwareNotSupportedErr;
}

OSStatus VDADecoderDecode()
{
	return kVDADecoderHardwareNotSupportedErr;
}

OSStatus VDADecoderFlush()
{
	return kVDADecoderHardwareNotSupportedErr;
}

