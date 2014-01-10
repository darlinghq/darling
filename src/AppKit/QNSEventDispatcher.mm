#include "QNSEventDispatcher.h"
#include <cstdint>
#include <QTimerEvent>
#include <QCoreApplication>
#include <QSocketNotifier>
#include <Foundation/NSInvocation.h>
#include <QPair>
#include "NSX11Event.h"

@interface NSStream (Private)
- (void) _setLoopID: (void *)ref;
@end

@implementation NSTimerInvocation
-(id) initWithEventDispatcher: (QNSEventDispatcher*) disp
{
	self = [self init];
	self->m_disp = disp;

	return self;
}

-(void) timerFired: (int)timerId
{
	self->m_disp->fireTimer(timerId);
}
@end

@implementation QtSocketDelegate
- (id)initWithSocketNotifier: (QSocketNotifier*)notifier
{
	self = [self init];
	self->m_notifier = notifier;
	return self;
}

- (void)stream:(NSStream *)theStream handleEvent:(NSStreamEvent)streamEvent
{
	/*
	int socket = [theStream _loopID];
	QSocketNotifier::Type type;

	switch (streamEvent)
	{
	case NSStreamEventOpenCompleted:
	case NSStreamEventHasSpaceAvailable:
		type = QSocketNotifier::Write;
		break;
	case NSStreamEventHasBytesAvailable:
	//case NSStreamEventErrorOccurred:
	case NSStreamEventEndEncountered:
		type = QSocketNotifier::Read;
		break;
	case NSStreamEventErrorOccurred:
		type = QSocketNotifier::Exception;
		break;
	}
*/

	QEvent event(QEvent::SockAct);
	QCoreApplication::sendEvent(self->m_notifier, &event);
}
@end

QNSEventDispatcher* QNSEventDispatcher::instance()
{
	static QNSEventDispatcher instance;
	return &instance;
}

QNSEventDispatcher::QNSEventDispatcher()
{
	m_runLoop = [NSRunLoop currentRunLoop];
	m_timerInvocation = [[NSTimerInvocation alloc] initWithEventDispatcher: this];

	setEventFilter(captureAllFilter);
}

QNSEventDispatcher::~QNSEventDispatcher()
{
	// kill all timers
	[m_runLoop release];
	[m_timerInvocation release];
}

bool QNSEventDispatcher::captureAllFilter(void* msg)
{
	XEvent* ev = static_cast<XEvent*>(msg);
	instance()->m_events.enqueue(*ev);
	return true;
}

void QNSEventDispatcher::flush()
{
}

bool QNSEventDispatcher::hasPendingEvents()
{
	extern Q_CORE_EXPORT uint qGlobalPostedEventsCount();
	return qGlobalPostedEventsCount() > 0;
}

void QNSEventDispatcher::interrupt()
{
}

bool QNSEventDispatcher::processEvents(QEventLoop::ProcessEventsFlags flags)
{
	emit awake();
	
	QCoreApplication::sendPostedEvents(0, -1);
	
	if (flags & QEventLoop::ExcludeSocketNotifiers)
		; // TODO
	if (flags & QEventLoop::X11ExcludeTimers)
		; // TODO
	
	[m_runLoop runUntilDate: [NSDate distantPast]];
	QCoreApplication::sendPostedEvents(0, -1);
	
	return true;
}

void QNSEventDispatcher::registerSocketNotifier(QSocketNotifier* notifier)
{
	int socket = notifier->socket();
	MySocketInfo info;
	
	switch (notifier->type())
	{
	case QSocketNotifier::Read:
	case QSocketNotifier::Exception:
		info.stream = [NSInputStream new];
		break;
	case QSocketNotifier::Write:
		info.stream = [NSOutputStream new];
		break;
	default:
		return;
	}

	info.delegate = [[QtSocketDelegate alloc] initWithSocketNotifier: notifier];

	m_sockets[notifier] = info;

	[info.stream _setLoopID: (void*)(intptr_t)socket];
	[info.stream setDelegate: (id)info.delegate];
	[info.stream scheduleInRunLoop: m_runLoop
	                  forMode: NSDefaultRunLoopMode];
}

void QNSEventDispatcher::registerTimer(int timerId, int interval, QObject* object)
{
	NSTimer* timer;
	MyTimerInfo info = { interval, object };
	NSInvocation* invocation = [NSInvocation new];

	[invocation setTarget: m_timerInvocation];
	[invocation setSelector: @selector(timerFired:)];
	[invocation setArgument: &timerId
	                atIndex: 0];

	timer = [NSTimer timerWithTimeInterval: double(interval)/1000.0
	                            invocation: invocation
	                               repeats: true];

	[invocation release];

	m_timers[timerId] = info;
	m_nsTimers[timerId] = timer;

	[m_runLoop addTimer: timer
	            forMode: NSDefaultRunLoopMode];
}

void QNSEventDispatcher::fireTimer(int timerId)
{
	QTimerEvent ev(timerId);
	QCoreApplication::sendEvent(m_timers[timerId].target, &ev);
}

QList<QAbstractEventDispatcher::TimerInfo> QNSEventDispatcher::registeredTimers(QObject* object) const
{
	QList<TimerInfo> rv;

	for (auto it = m_timers.begin(); it != m_timers.end(); it++)
	{
		if (it.value().target == object)
			rv << TimerInfo(it.key(), it.value().interval);
	}

	return rv;
}

void QNSEventDispatcher::unregisterSocketNotifier(QSocketNotifier* notifier)
{
	if (!m_sockets.contains(notifier))
		return;

	MySocketInfo& info = m_sockets[notifier];
	[info.stream removeFromRunLoop: m_runLoop
	                       forMode: NSDefaultRunLoopMode];
	[info.stream release];
	[info.delegate release];

	m_sockets.remove(notifier);
}

bool QNSEventDispatcher::unregisterTimer(int timerId)
{
	if (!m_nsTimers.contains(timerId))
		return false;

	NSTimer* timer = m_nsTimers.take(timerId);
	[timer invalidate];
	[timer release];
	m_nsTimers.remove(timerId);
	m_timers.remove(timerId);

	return true;
}

bool QNSEventDispatcher::unregisterTimers(QObject* object)
{
	QList<int> ids;
	bool rv = false;

	for (auto it = m_timers.begin(); it != m_timers.end(); it++)
	{
		if (it.value().target == object)
			ids << it.key();
			//rv |= unregisterTimer(it.key());
	}

	for (int _id : ids)
		rv |= unregisterTimer(_id);

	return rv;
}

void QNSEventDispatcher::wakeUp()
{
}

NSEvent* QNSEventDispatcher::peekEvent()
{
	XEvent ev = m_events.head();
	return [NSEvent eventWithCGEvent: &ev];
}

NSEvent* QNSEventDispatcher::takeEvent()
{
	XEvent ev = m_events.dequeue();
	return [NSEvent eventWithCGEvent: &ev];
}

