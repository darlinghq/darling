#ifndef APPLECFSTRING_H
#define APPLECFSTRING_H
#include <stdint.h>
#include <limits.h>
#include <CoreFoundation/CFString.h>
#include <Foundation/NSString.h>

#pragma pack(1)
@interface AppleCFString : NSString
{
	uint8_t _flags;
	uint8_t _typeID;
	uint8_t _unused[2];
#ifdef __x86_64__
	uint32_t _extraFlags;
#endif
	union
	{
		const unichar* _unicodeData;
		const char* _asciiData;
	} _data;
	uintptr_t _length;
};

- (NSUInteger)length;
- (unichar)characterAtIndex:(NSUInteger)index;
- (void)getCharacters:(unichar *)buffer range:(NSRange)aRange;
- (const char *)UTF8String;

- (NSUInteger) retainCount;
- (id) retain;
- (void) release;
- (id) autorelease;
- (NSZone*) zone;
- (CFTypeID) _cfTypeID;
@end
#pragma pack()

enum { apple_immutable = 0x80, apple_notinline = 0x40, apple_unicode = 0x20, apple_hasnull = 0x10, apple_haslength = 8 };

#endif

