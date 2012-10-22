#ifndef NSCFSTRING_H
#define NSCFSTRING_H
#include <Foundation/NSString.h>

@interface NSCFString : NSString
{
	unsigned long info;
	const char* nxcsptr;
	unsigned long nxcslen;
}

@end

#endif
