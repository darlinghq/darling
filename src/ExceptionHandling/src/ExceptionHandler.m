#include "ExceptionHandler.h"
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <cstdlib>

static void LocalExceptionHandler(NSException* e);

@implementation NSExceptionHandler

@synthesize exceptionHandlingMask = _handlingMask;
@synthesize exceptionHangingMask = _hangingMask;
@synthesize delegate = _delegate;

+ (NSExceptionHandler *)defaultExceptionHandler
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	static NSExceptionHandler* instance;
	
	pthread_once(&once, []() {
	
		instance = [NSExceptionHandler new];
		instance->_delegate = NULL;
		instance->_handlingMask = 0xffffffff;
		instance->_hangingMask = 0;
		
		NSSetUncaughtExceptionHandler(LocalExceptionHandler);
	});
	
	return instance;
}

@end

void LocalExceptionHandler(NSException* e)
{
	NSExceptionHandler* handler = [NSExceptionHandler defaultExceptionHandler];
	unsigned int mask = handler.exceptionHandlingMask;
	unsigned int hangmask = handler.exceptionHangingMask;
	id delegate = handler.delegate;

	if (delegate != NULL)
	{
		bool handle, log;

		handle = [delegate exceptionHandler: [NSExceptionHandler defaultExceptionHandler]
		              shouldHandleException: e
                                       mask: mask];

		log = [delegate exceptionHandler: [NSExceptionHandler defaultExceptionHandler]
		              shouldLogException: e
                                    mask: mask];

		if (log)
			NSLog(@"Uncaught exception: %@", e);
		if (handle)
			abort();
	}
	else
	{
		if (mask & NSLogUncaughtExceptionMask)
			NSLog(@"Uncaught exception: %@", e);
		if (hangmask & NSHangOnUncaughtExceptionMask)
			kill(getpid(), SIGTRAP);
		if (mask & NSHandleUncaughtExceptionMask)
			abort();
	}
}
