#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include "../CFF/CFStringConst.h"

// A real implementation would use VA API or VDPAU

CONST_STRING_DECL(kVDADecoderConfiguration_Height, "kVDADecoderConfiguration_Height");
CONST_STRING_DECL(kVDADecoderConfiguration_Width, "kVDADecoderConfiguration_Width");
CONST_STRING_DECL(kVDADecoderConfiguration_SourceFormat, "kVDADecoderConfiguration_SourceFormat");
CONST_STRING_DECL(kVDADecoderConfiguration_avcCData, "kVDADecoderConfiguration_avcCData");

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

