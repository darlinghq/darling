#include <AppKit/NSResponder.h>

@implementation NSResponder
- (NSResponder *)nextResponder
{
	return _nextResponder;
}

- (void)setNextResponder:(NSResponder *)aResponder
{
	_nextResponder = aResponder;
}

- (void)noResponderFor:(SEL)eventSelector
{
}

- (BOOL)acceptsFirstResponder
{
	return false;
}

- (BOOL)becomeFirstResponder
{
	return true;
}

- (BOOL)resignFirstResponder
{
	return true;
}

- (BOOL)tryToPerform:(SEL)anAction with:(id)anObject
{
	if ([self respondsToSelector: anAction])
	{
		objc_msgSend(self, anAction, anObject);
		return true;
	}
	else if (_nextResponder != nullptr)
		return [_nextResponder tryToPerform: anAction with: anObject];
	else
		return false;
}

- (NSUndoManager *)undoManager
{
	if (_nextResponder)
		return [_nextResponder undoManager];
	else
	{
		[self noResponderFor: @selector(undoManager)];
		return nullptr;
	}
}

- (void)setMenu:(NSMenu *)menu
{
	[self noResponderFor: @selector(setMenu:)];
}

- (NSMenu *)menu
{
	[self noResponderFor: @selector(menu)];
	return nullptr;
}

- (BOOL)performKeyEquivalent:(NSEvent *)theEvent
{
	return false;
}

#define PASS_EVENT() if (_nextResponder) objc_msgSend(_nextResponder, _cmd, theEvent); else [self noResponderFor: _cmd]

- (void)mouseDown:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)otherMouseDown:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)mouseUp:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)otherMouseUp:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)mouseMoved:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	PASS_EVENT();
}


- (void)scrollWheel:(NSEvent *)theEvent
{
	PASS_EVENT();
}


- (void)rightMouseDragged:(NSEvent *)theEvent
{
	PASS_EVENT();
}


- (void)otherMouseDragged:(NSEvent *)theEvent
{
	PASS_EVENT();
}


- (void)mouseEntered:(NSEvent *)theEvent
{
	PASS_EVENT();
}


- (void)mouseExited:(NSEvent *)theEvent
{
	PASS_EVENT();
}


- (void)keyDown:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)keyUp:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)flagsChanged:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)tabletPoint:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)tabletProximity:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)cursorUpdate:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)magnifyWithEvent:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)rotateWithEvent:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)swipeWithEvent:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)beginGestureWithEvent:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)endGestureWithEvent:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)smartMagnifyWithEvent:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)touchesBeganWithEvent:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)touchesMovedWithEvent:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)touchesEndedWithEvent:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)touchesCancelledWithEvent:(NSEvent *)theEvent
{
	PASS_EVENT();
}

- (void)quickLookWithEvent:(NSEvent *)theEvent
{
	PASS_EVENT();
}


@end
