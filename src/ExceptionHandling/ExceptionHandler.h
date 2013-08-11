#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H
#include <Foundation/NSException.h>

@interface NSExceptionHandler : NSObject
{
	@private
	id _delegate;
	unsigned int _handlingMask, _hangingMask;
}

+ (NSExceptionHandler *)defaultExceptionHandler;

@property (nonatomic) unsigned int exceptionHandlingMask;
@property (nonatomic) unsigned int exceptionHangingMask;
@property (nonatomic, retain) id delegate;

@end

enum
{
   NSHangOnUncaughtExceptionMask = 1,
   NSHangOnUncaughtSystemExceptionMask = 2,
   NSHangOnUncaughtRuntimeErrorMask = 4,
   NSHangOnTopLevelExceptionMask = 8,
   NSHangOnOtherExceptionMask = 16
};

enum
{
   NSLogUncaughtExceptionMask            = 1,
   NSHandleUncaughtExceptionMask         = 2,
   NSLogUncaughtSystemExceptionMask      = 4,
   NSHandleUncaughtSystemExceptionMask   = 8,
   NSLogUncaughtRuntimeErrorMask         = 0x10,
   NSHandleUncaughtRuntimeErrorMask      = 0x20,
   NSLogTopLevelExceptionMask            = 0x40,
   NSHandleTopLevelExceptionMask         = 0x80,
   NSLogOtherExceptionMask               = 0x100,
   NSHandleOtherExceptionMask            = 0x200
};

#endif
