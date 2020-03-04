#ifndef _Carbon_Menus_H_
#define _Carbon_Menus_H_

#include <CoreServices/MacTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpaqueMenuRef* MenuRef;
typedef UInt16	MenuItemIndex;

void ShowMenuBar(void);
void HideMenuBar(void);

#ifdef __cplusplus
}
#endif

#endif
