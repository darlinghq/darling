#ifndef HITOOLBOX_KEYBOARDS_H
#define HITOOLBOX_KEYBOARDS_H

#include "CoreServices/MacTypes.h"

#ifdef __cplusplus
extern "C" {
#endif


enum {
	kKeyboardJIS                  = 'JIS ',
	kKeyboardANSI                 = 'ANSI',
	kKeyboardISO                  = 'ISO ',
	kKeyboardUnknown              =  kUnknownType /* ???? */
};

typedef OSType PhysicalKeyboardLayoutType;

extern PhysicalKeyboardLayoutType KBGetLayoutType(SInt16 iKeyboardType);

#ifdef __cplusplus
}
#endif

#endif
