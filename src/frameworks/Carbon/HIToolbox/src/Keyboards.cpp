#include <HIToolbox/Keyboards.h>

PhysicalKeyboardLayoutType KBGetLayoutType(SInt16 iKeyboardType)
{
	// We behave like ANSI
	return kKeyboardANSI;
}
