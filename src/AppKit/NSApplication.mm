#include <AppKit/NSApplication.h>
#include "NSQEvent.h"
#include <Foundation/NSRunLoop.h>
#include <Foundation/NSProcessInfo.h>
#include <Foundation/NSBundle.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSNotification.h>
#include <QVector>
#include <QQueue>
#include <unistd.h>
#include <mutex>
#include <memory>

id NSApp = nullptr;
const double NSAppKitVersionNumber = NSAppKitVersionNumber10_7_2;

DEFINE_CONSTANT(NSApplicationDidBecomeActiveNotification);
DEFINE_CONSTANT(NSApplicationDidHideNotification);
DEFINE_CONSTANT(NSApplicationDidFinishLaunchingNotification);
DEFINE_CONSTANT(NSApplicationDidResignActiveNotification);
DEFINE_CONSTANT(NSApplicationDidUnhideNotification);
DEFINE_CONSTANT(NSApplicationDidUpdateNotification);
DEFINE_CONSTANT(NSApplicationWillBecomeActiveNotification);
DEFINE_CONSTANT(NSApplicationWillHideNotification);
DEFINE_CONSTANT(NSApplicationWillFinishLaunchingNotification);
DEFINE_CONSTANT(NSApplicationWillResignActiveNotification);
DEFINE_CONSTANT(NSApplicationWillUnhideNotification);
DEFINE_CONSTANT(NSApplicationWillUpdateNotification);
DEFINE_CONSTANT(NSApplicationWillTerminateNotification);
DEFINE_CONSTANT(NSApplicationDidChangeScreenParametersNotification);


class NSApplicationEventFilter : public QObject
{
public:
	NSApplicationEventFilter(QObject* parent) : QObject(parent) {}

	std::shared_ptr<QEvent> takeEvent(WId& wid)
	{
		if (m_queue.isEmpty())
			return nullptr;
		else
		{
			auto e = m_queue.dequeue();
			wid = e.second;
			return e.first;
		}
	}
	
	std::shared_ptr<QEvent> peekEvent(WId& wid)
	{
		if (m_queue.isEmpty())
			return nullptr;
		else
		{
			auto e = m_queue.head();
			wid = e.second;
			return e.first;
		}
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
		// Let the other events pass.

		QWindow* targetWin = qobject_cast<QWindow*>(obj);

		if (targetWin != nullptr)
		{
			QEvent* clone = cloneEvent(event);

			if (clone)
			{
				m_queue.enqueue(QPair<std::shared_ptr<QEvent>, WId>(std::shared_ptr<QEvent>(clone), targetWin->winId()));
				return true;
			}
		}
		return QObject::eventFilter(obj, event);
	}
private:
	QQueue<QPair<std::shared_ptr<QEvent>, WId>> m_queue;
};

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
	
	m_delegate = nullptr;
	
	@autoreleasepool
	{
		NSArray* args = [[NSProcessInfo processInfo] arguments];
		QVector<char*> qargs;
		int argc = [args count];
		
		for (NSString* arg in args)
			qargs << (char*) [arg UTF8String];
		
		qargs << nullptr;
		
		m_running = false;
		QCoreApplication::setEventDispatcher(QNSEventDispatcher::instance());
		m_application = new QGuiApplication(argc, qargs.data());
		m_eventFilter = new NSApplicationEventFilter(m_application);
		m_application->installEventFilter(m_eventFilter);
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

- (void) setDelegate:(id<NSApplicationDelegate>)anObject
{
	NSNotificationCenter* nc = [NSNotificationCenter defaultCenter];
	
	if (m_delegate)
		[nc removeObserver: m_delegate name: nil object: self];
		
	m_delegate = anObject;
	
	if (m_delegate != nullptr)
	{
#define ADD_OBSERVER(event) \
		[nc addObserver: m_delegate \
			   selector: @selector(application##event) \
				   name: NSApplication##event##Notification \
				 object: self]
		
		ADD_OBSERVER(DidBecomeActive);
		ADD_OBSERVER(DidHide);
		ADD_OBSERVER(DidFinishLaunching);
		ADD_OBSERVER(DidResignActive);
		ADD_OBSERVER(DidUnhide);
		ADD_OBSERVER(DidUpdate);
		ADD_OBSERVER(WillBecomeActive);
		ADD_OBSERVER(WillHide);
		ADD_OBSERVER(WillFinishLaunching);
		ADD_OBSERVER(WillResignActive);
		ADD_OBSERVER(WillUnhide);
		ADD_OBSERVER(WillUpdate);
		ADD_OBSERVER(WillTerminate);
		ADD_OBSERVER(DidChangeScreenParameters);
		
#undef ADD_OBSERVER
	}
}

- (id<NSApplicationDelegate>) delegate
{
	return m_delegate;
}

-(void) stop: (id) sender
{
	m_running = false;
}

-(void) run
{
	NSDate* distantFuture = [NSDate distantFuture];
	NSNotificationCenter* nc = [NSNotificationCenter defaultCenter];
	
	[nc postNotificationName: NSApplicationWillFinishLaunchingNotification
	                  object: self];
	                  
	// TODO: open file notifications etc.
	
	[nc postNotificationName: NSApplicationDidFinishLaunchingNotification
	                  object: self];
	
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
	std::shared_ptr<QEvent> qevent;
	WId wid;
	
	if (expiration != nullptr)
		interval = [expiration timeIntervalSinceNow];
	
	if (interval*1000 < INT_MAX)
		m_application->processEvents(QEventLoop::AllEvents, int(interval * 1000));
	else
		m_application->processEvents(QEventLoop::AllEvents);
	
	if (flag)
		qevent = m_eventFilter->takeEvent(wid);
	else
		qevent = m_eventFilter->peekEvent(wid);
	
	event = [[NSQEvent alloc] initWithQEvent: qevent WId: wid];
	
	return [event autorelease];
}

- (void)sendEvent:(NSEvent *)anEvent
{
	QEvent* qevent = [anEvent CGEvent];
	WId wid = [anEvent windowNumber];

	if (qevent != nullptr)
	{
		// It is a wrapped X11 event
		m_application->sendEvent(QWindow::fromWinId(wid), qevent);
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


