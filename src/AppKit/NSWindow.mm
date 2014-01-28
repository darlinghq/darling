#include <AppKit/NSWindow.h>
#include "Util.h"
#include <Foundation/NSURL.h>
#include <AppKit/NSScreen.h>
#include <CoreGraphics/CGGeometry_p.h>

@implementation NSWindow
- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag
{
	return [self initWithContentRect: contentRect styleMask: aStyle backing: bufferingType defer: flag screen: nullptr];
}
- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag screen:(NSScreen *)screen
{
	QScreen* qscreen = nullptr;
	int qflags = Qt::CustomizeWindowHint;
	QRect frame;
	int height;

	if (screen)
	{
		qscreen = screen->_screen;
		height = qscreen->size().height();
	}
	else
		height = static_cast<QGuiApplication*>(qApp)->primaryScreen()->size().height();
	frame = QRectFromNSRect(contentRect, height);

	if (aStyle & NSTitledWindowMask)
		qflags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint;
	if (aStyle & NSMiniaturizableWindowMask)
		qflags |= Qt::WindowMinimizeButtonHint;
	if (aStyle & NSClosableWindowMask)
		qflags |= Qt::WindowCloseButtonHint;
	
	self = [self init];

	_opacity = 1;
	_releasedWhenClosed = true; // TODO: implement release unless controller is present
	_window = std::shared_ptr<QQuickWindow>(new QQuickWindow);
	_window->setGeometry(frame);

	if (!(aStyle & NSResizableWindowMask))
	{
		_window->setMaximumSize(_window->baseSize());
		_window->setMinimumSize(_window->baseSize());
	}

	return self;
}

- (NSString *)title
{
	return NSStringFromQString(_window->title());
}

- (void)setTitle:(NSString *)aString
{
	_window->setTitle(QStringFromNSString(aString));
}

- (NSInteger)windowNumber
{
	return _window->winId();
}

- (NSRect)frame
{
	int height = _window->screen()->size().height();
	return NSRectFromQRect(_window->frameGeometry(), height);
}

- (void)makeKeyAndOrderFront:(id)sender
{
	_window->show();
	_window->requestActivate();
}

- (void)orderFront:(id)sender
{
	_window->show();
}

- (void)orderOut:(id)sender
{
	_window->hide();
}

- (void)setRepresentedURL:(NSURL *)url
{
	[self setRepresentedFilename: [url path]];
}

- (NSURL *)representedURL
{
	return [NSURL fileURLWithPath: [self representedFilename]];
}

- (NSString *)representedFilename
{
	return NSStringFromQString(_window->filePath());
}

- (void)setRepresentedFilename:(NSString *)aString
{
	_window->setFilePath(QStringFromNSString(aString));
}

- (BOOL)isVisible
{
	return _window->isVisible();
}

- (BOOL)isKeyWindow
{
	return _window->isActive();
}

- (BOOL)isMainWindow
{
	return true; // TODO
}

- (BOOL)canBecomeKeyWindow
{
	return true;
}

- (BOOL)canBecomeMainWindow
{
	return true;
}

- (void)makeKeyWindow
{
	_window->requestActivate();
}

- (void)makeMainWindow
{
	// TODO
	[self becomeMainWindow];
}

- (void)becomeKeyWindow
{
	// TODO: subclass QQuickWindow and invoke this from event handlers
	// focusInEvent
}

- (void)resignKeyWindow
{
	// focusOutEvent
}

- (void)becomeMainWindow
{
}

- (void)resignMainWindow
{
}

- (void)toggleFullScreen:(id)sender
{
	if (_window->isVisible()) // TODO: test if this 'if' is correct
	{
		if (_window->windowState() & Qt::WindowFullScreen)
			_window->showNormal();
		else
			_window->showFullScreen();
	}
}

- (NSSize)minSize
{
	return [self contentMinSize]; // TODO: recalculate
}

- (NSSize)maxSize
{
	return [self contentMaxSize]; // TODO: recalculate
}

- (void)setMinSize:(NSSize)size
{
	[self setContentMinSize: size]; // TODO: recalculate
}

- (void)setMaxSize:(NSSize)size
{
	[self setContentMaxSize: size]; // TODO: recalculate
}

- (NSSize)contentMinSize
{
	return NSSizeFromQSize(_window->minimumSize());
}

- (NSSize)contentMaxSize
{
	return NSSizeFromQSize(_window->maximumSize());
}

- (void)setContentMinSize:(NSSize)size
{
	_window->setMinimumSize(QSizeFromNSSize(size));
}

- (void)setContentMaxSize:(NSSize)size
{
	_window->setMaximumSize(QSizeFromNSSize(size));
}

- (void)setAlphaValue:(CGFloat)windowAlpha
{
	_opacity = windowAlpha;
	_window->setOpacity(windowAlpha);
}

- (CGFloat)alphaValue
{
	return _opacity;
}

- (void)setOpaque:(BOOL)isOpaque
{
	if (isOpaque)
		[self setAlphaValue: 1];
	else
		[self setAlphaValue: 0.5]; // what's the right value?
}

- (BOOL)isOpaque
{
	return _opacity == 1;
}

- (void)setReleasedWhenClosed:(BOOL)flag
{
	_releasedWhenClosed = flag;
}

- (BOOL)isReleasedWhenClosed
{
	return _releasedWhenClosed;
}

@end
