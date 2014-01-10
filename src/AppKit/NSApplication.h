#ifndef NSAPPLICATION_H
#define NSAPPLICATION_H
#include "AppKit.h"
#include <Foundation/NSObject.h>

#ifdef DARLING_BUILD
#	include <QEventLoop>
#	include <QApplication>
#	include "QNSEventDispatcher.h"

class NSApplicationEventFilter;
#endif

@class NSEvent;

@interface NSApplication : NSObject
#ifdef DARLING_BUILD
{
	bool m_running;
	QApplication* m_application;
}
#endif

+(NSApplication*) sharedApplication;
-(id) init;
-(void) dealloc;

-(BOOL) isRunning;
-(void) stop: (id) sender;
-(void) run;

- (NSEvent *)nextEventMatchingMask:(NSUInteger)mask untilDate:(NSDate *)expiration inMode:(NSString *)mode dequeue:(BOOL)flag;

- (void)sendEvent:(NSEvent *)anEvent;

@end

BEGIN_EXTERN_C

extern id NSApp;

BOOL NSApplicationLoad(void);
int NSApplicationMain(int argc, const char *argv[]);

END_EXTERN_C

#endif

