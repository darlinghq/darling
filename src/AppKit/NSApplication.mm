#include "NSApplication.h"
#include "NSX11Event.h"
#include <Foundation/NSRunLoop.h>
#include <Foundation/NSProcessInfo.h>
#include <Foundation/NSBundle.h>
#include <Foundation/NSDictionary.h>
#include <QVector>
#include <QQueue>
#include <unistd.h>
#include <mutex>
#include <memory>

id NSApp = nullptr;

/*
class NSApplicationEventFilter : public QObject
{
public:
	std::shared_ptr<QEvent> takeEvent()
	{
		if (m_queue.isEmpty())
			return nullptr;
		else
			return m_queue.dequeue();
	}
	
	std::shared_ptr<QEvent> peekEvent()
	{
		if (m_queue.isEmpty())
			return nullptr;
		else
			return m_queue.head();
	}
private:
	static QEvent* cloneEvent(QEvent* e)
	{
		switch (e->type())
		{
			//case 
			default:
				return nullptr;
		}
	}
protected:
	bool eventFilter(QObject* obj, QEvent* event) override
	{
		// TODO: Manually create clones of all events that may happen in this application and enqueue them.
		// Let the other functions pass.
		QEvent* clone = cloneEvent(event);

		if (!clone)
			return QObject::eventFilter(obj, event);
		else
			return true;
	}
private:
	QQueue<std::shared_ptr<QEvent>> m_queue;
};
*/

@implementation NSApplication
+(NSApplication*) sharedApplication
{
	static std::mutex mutex;
	
	if (NSApp != nullptr)
		return NSApp;
	
	std::lock_guard<std::mutex> lock(mutex);
	if (NSApp == nullptr)
		NSApp = [self new];
	return NSApp;
}

-(id) init
{
	self = [super init];
	
	@autoreleasepool
	{
		NSArray* args = [[NSProcessInfo processInfo] arguments];
		QVector<char*> qargs;
		int argc = [args count];
		
		for (NSString* arg in args)
			qargs << (char*) [arg UTF8String];
		
		qargs << nullptr;
		
		m_running = false;
		QNSEventDispatcher::instance(); // force singleton creation
		m_application = new QApplication(argc, qargs.data());
	}
	return self;
}

-(void) dealloc
{
	delete m_application;
	[super dealloc];
}

-(BOOL) isRunning
{
	return m_running;
}

-(void) stop: (id) sender
{
	m_running = false;
}

-(void) run
{
	NSDate* distantFuture = [NSDate distantFuture];
	m_running = true;
	
	while (m_running)
	{
		@autoreleasepool
		{
			@try
			{
				NSEvent* event;
				
				event = [self nextEventMatchingMask: NSAnyEventMask
										  untilDate: distantFuture
											 inMode: NSDefaultRunLoopMode
											dequeue: true];
				
				if (event != nullptr)
					[self sendEvent: event];
			}
			@catch (NSException* e)
			{
				// TODO: handle exceptions
			}
		}
	}
}

- (NSEvent *)nextEventMatchingMask:(NSUInteger)mask untilDate:(NSDate *)expiration inMode:(NSString *)mode dequeue:(BOOL)flag
{
	// TODO: mask and other arguments!
	NSTimeInterval interval = 0;
	NSEvent* event;
	
	if (expiration != nullptr)
		interval = [expiration timeIntervalSinceNow];
	
	if (interval*1000 < INT_MAX)
		m_application->processEvents(QEventLoop::AllEvents, int(interval * 1000));
	else
		m_application->processEvents(QEventLoop::AllEvents);
	
	if (flag)
		event = QNSEventDispatcher::instance()->takeEvent();
	else
		event = QNSEventDispatcher::instance()->peekEvent();
	
	return event;
}

- (void)sendEvent:(NSEvent *)anEvent
{
	XEvent* event = [anEvent CGEvent];

	if (event != nullptr)
	{
		// It is a wrapped X11 event
		m_application->x11ProcessEvent(event);
	}
	else
	{
		// It is an application-generated event
		// TODO: Wrap it and pass it through Qt's event system
	}
}
@end

BOOL NSApplicationLoad(void)
{
	return true;
}

int NSApplicationMain(int argc, const char *argv[])
{
	@autoreleasepool
	{
		NSDictionary* dict;
		NSString* principialClassName;
		Class principialClass;
		
		[NSProcessInfo initializeWithArguments: (char**) argv
										count: argc
								environment: environ];
		
		dict = [[NSBundle mainBundle] infoDictionary];
		principialClassName = [dict objectForKey: @"NSPrincipialClass"];
		principialClass = NSClassFromString(principialClassName);
		
		[principialClass sharedApplication];
		
		// TODO: load main NIB
	}
	
	@autoreleasepool
	{
		[NSApp run];
		[NSApp release];
	}
	
	return 0;
}


