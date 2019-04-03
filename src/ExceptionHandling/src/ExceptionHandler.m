#include <ExceptionHandling/ExceptionHandler.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

NSString *NSStackTraceKey = @"NSStackTraceKey";

static void LocalExceptionHandler(NSException* e);

@implementation NSExceptionHandler

@synthesize exceptionHandlingMask = _handlingMask;
@synthesize exceptionHangingMask = _hangingMask;
@synthesize delegate = _delegate;

static NSExceptionHandler* instance;

static void init_routine(void)
{
	instance = [NSExceptionHandler new];
	instance->_delegate = NULL;
	instance->_handlingMask = 0xffffffff;
	instance->_hangingMask = 0;

	NSSetUncaughtExceptionHandler(LocalExceptionHandler);
}

+ (NSExceptionHandler *)defaultExceptionHandler
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	
	pthread_once(&once, &init_routine);

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
