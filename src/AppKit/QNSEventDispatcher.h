#ifndef QNSEVENTDISPATCHER_H
#define QNSEVENTDISPATCHER_H
#include <QtCore/qabstracteventdispatcher.h>
#include <QHash>
#include <QQueue>
#include <QTimerEvent>
#include <Foundation/NSRunLoop.h>
#include <Foundation/NSTimer.h>
#include <Foundation/NSStream.h>
#include <AppKit/NSEvent.h>

class QNSEventDispatcher;
@interface NSTimerInvocation : NSObject
{
	QNSEventDispatcher* m_disp;
}

-(id) initWithEventDispatcher: (QNSEventDispatcher*) disp;
-(void) timerFired: (int)timerId;
@end

@interface QtSocketDelegate : NSObject //<NSStreamDelegate> // not until very recent gnustep-base code
{
	QSocketNotifier* m_notifier;
}
- (id)initWithSocketNotifier: (QSocketNotifier*)notifier;
- (void)stream:(NSStream *)theStream handleEvent:(NSStreamEvent)streamEvent;
@end

class QNSEventDispatcher : public QAbstractEventDispatcher
{
private:
	QNSEventDispatcher();
	~QNSEventDispatcher();
public:
	static QNSEventDispatcher* instance();

	virtual void flush() override;
	virtual bool hasPendingEvents() override;
	virtual void interrupt() override;
	virtual bool processEvents(QEventLoop::ProcessEventsFlags flags) override;
	virtual void registerSocketNotifier(QSocketNotifier* notifier) override;
	virtual void registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject* object) override;
	virtual QList<TimerInfo> registeredTimers(QObject* object) const override;
	virtual void unregisterSocketNotifier(QSocketNotifier* notifier) override;
	virtual bool unregisterTimer(int timerId) override;
	virtual bool unregisterTimers(QObject* object) override;
	virtual void wakeUp() override;
	virtual int remainingTime(int timerId) override;

	void fireTimer(int timerId);
private:
	struct MyTimerInfo
	{
		int interval;
		Qt::TimerType type;
		QObject* target;
	};
	struct MySocketInfo
	{
		QtSocketDelegate* delegate;
		NSStream* stream;	
	};

	NSRunLoop* m_runLoop;
	QHash<int, NSTimer*> m_nsTimers;
	QHash<int, MyTimerInfo> m_timers;
	QHash<QSocketNotifier*, MySocketInfo> m_sockets;
	NSTimerInvocation* m_timerInvocation;
};

#endif

