#import "AppleCFString.h"
#import <Foundation/NSException.h>
#include <string.h>

extern "C" Class* _OBJC_CLASS_AppleCFString;
extern "C" Class* __CFConstantStringClassReference;

Class* __CFConstantStringClassReference  __attribute__ ((weak, alias ("_OBJC_CLASS_AppleCFString")));

@implementation AppleCFString
- (NSUInteger)length
{
	return _length;
}

- (unichar)characterAtIndex:(NSUInteger)index
{
	if (index >= _length)
		[NSException raise: NSRangeException format: @"Invalid index"];

	if (_flags & apple_unicode)
		return _data._unicodeData[index];
	else
		return _data._asciiData[index];
}

- (void)getCharacters:(unichar *)buffer range:(NSRange)aRange
{
	if (aRange.location+aRange.length > _length)
		[NSException raise: NSRangeException format: @"Invalid range"];
	
	if (_flags & apple_unicode)
	{
		memcpy(buffer, _data._unicodeData + aRange.location, aRange.length);
	}
	else
	{
		for (int i = 0; i < aRange.length; i++)
			buffer[i] = _data._asciiData[i+aRange.location];
	}
}

- (const char *)UTF8String
{
	if (_flags & apple_unicode)
		return [super UTF8String];
	else
		return _data._asciiData;
}
@end

