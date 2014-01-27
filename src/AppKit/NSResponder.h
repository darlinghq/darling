#import <Foundation/NSObject.h>
#import <AppKit/NSEvent.h>

@class NSArray, NSError, NSEvent, NSMenu, NSUndoManager, NSWindow;

@interface NSResponder : NSObject <NSCoding>
{

    id _nextResponder;
}

- (NSResponder *)nextResponder;
- (void)setNextResponder:(NSResponder *)aResponder;
- (BOOL)tryToPerform:(SEL)anAction with:(id)anObject;
- (BOOL)performKeyEquivalent:(NSEvent *)theEvent;
- (id)validRequestorForSendType:(NSString *)sendType returnType:(NSString *)returnType;
- (void)mouseDown:(NSEvent *)theEvent;
- (void)rightMouseDown:(NSEvent *)theEvent;
- (void)otherMouseDown:(NSEvent *)theEvent;
- (void)mouseUp:(NSEvent *)theEvent;
- (void)rightMouseUp:(NSEvent *)theEvent;
- (void)otherMouseUp:(NSEvent *)theEvent;
- (void)mouseMoved:(NSEvent *)theEvent;
- (void)mouseDragged:(NSEvent *)theEvent;
- (void)scrollWheel:(NSEvent *)theEvent;
- (void)rightMouseDragged:(NSEvent *)theEvent;
- (void)otherMouseDragged:(NSEvent *)theEvent;
- (void)mouseEntered:(NSEvent *)theEvent;
- (void)mouseExited:(NSEvent *)theEvent;
- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;
- (void)flagsChanged:(NSEvent *)theEvent;
- (void)tabletPoint:(NSEvent *)theEvent;
- (void)tabletProximity:(NSEvent *)theEvent;
- (void)cursorUpdate:(NSEvent *)event;


- (void)magnifyWithEvent:(NSEvent *)event;
- (void)rotateWithEvent:(NSEvent *)event;
- (void)swipeWithEvent:(NSEvent *)event;
- (void)beginGestureWithEvent:(NSEvent *)event;
- (void)endGestureWithEvent:(NSEvent *)event;

- (void)smartMagnifyWithEvent:(NSEvent *)event;




- (void)touchesBeganWithEvent:(NSEvent *)event;



- (void)touchesMovedWithEvent:(NSEvent *)event;



- (void)touchesEndedWithEvent:(NSEvent *)event;



- (void)touchesCancelledWithEvent:(NSEvent *)event;


- (void)quickLookWithEvent:(NSEvent *)event;

- (void)noResponderFor:(SEL)eventSelector;
- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)resignFirstResponder;

- (void)interpretKeyEvents:(NSArray *)eventArray;
- (void)flushBufferedKeyEvents;

- (void)setMenu:(NSMenu *)menu;
- (NSMenu *)menu;

- (void)showContextHelp:(id)sender;

- (void)helpRequested:(NSEvent *)eventPtr;

- (BOOL)shouldBeTreatedAsInkEvent:(NSEvent *)theEvent;



- (BOOL)wantsScrollEventsForSwipeTrackingOnAxis:(NSEventGestureAxis)axis;



- (BOOL)wantsForwardedScrollEventsForAxis:(NSEventGestureAxis)axis;



- (id)supplementalTargetForAction:(SEL)action sender:(id)sender;
@end

@interface NSResponder (NSStandardKeyBindingMethods)





- (void)insertText:(id)insertString;


- (void)doCommandBySelector:(SEL)aSelector;






- (void)moveForward:(id)sender;
- (void)moveRight:(id)sender;
- (void)moveBackward:(id)sender;
- (void)moveLeft:(id)sender;
- (void)moveUp:(id)sender;
- (void)moveDown:(id)sender;
- (void)moveWordForward:(id)sender;
- (void)moveWordBackward:(id)sender;
- (void)moveToBeginningOfLine:(id)sender;
- (void)moveToEndOfLine:(id)sender;
- (void)moveToBeginningOfParagraph:(id)sender;
- (void)moveToEndOfParagraph:(id)sender;
- (void)moveToEndOfDocument:(id)sender;
- (void)moveToBeginningOfDocument:(id)sender;
- (void)pageDown:(id)sender;
- (void)pageUp:(id)sender;
- (void)centerSelectionInVisibleArea:(id)sender;

- (void)moveBackwardAndModifySelection:(id)sender;
- (void)moveForwardAndModifySelection:(id)sender;
- (void)moveWordForwardAndModifySelection:(id)sender;
- (void)moveWordBackwardAndModifySelection:(id)sender;
- (void)moveUpAndModifySelection:(id)sender;
- (void)moveDownAndModifySelection:(id)sender;

- (void)moveToBeginningOfLineAndModifySelection:(id)sender;
- (void)moveToEndOfLineAndModifySelection:(id)sender;
- (void)moveToBeginningOfParagraphAndModifySelection:(id)sender;
- (void)moveToEndOfParagraphAndModifySelection:(id)sender;
- (void)moveToEndOfDocumentAndModifySelection:(id)sender;
- (void)moveToBeginningOfDocumentAndModifySelection:(id)sender;
- (void)pageDownAndModifySelection:(id)sender;
- (void)pageUpAndModifySelection:(id)sender;
- (void)moveParagraphForwardAndModifySelection:(id)sender;
- (void)moveParagraphBackwardAndModifySelection:(id)sender;

- (void)moveWordRight:(id)sender;
- (void)moveWordLeft:(id)sender;
- (void)moveRightAndModifySelection:(id)sender;
- (void)moveLeftAndModifySelection:(id)sender;
- (void)moveWordRightAndModifySelection:(id)sender;
- (void)moveWordLeftAndModifySelection:(id)sender;

- (void)moveToLeftEndOfLine:(id)sender;
- (void)moveToRightEndOfLine:(id)sender;
- (void)moveToLeftEndOfLineAndModifySelection:(id)sender;
- (void)moveToRightEndOfLineAndModifySelection:(id)sender;

- (void)scrollPageUp:(id)sender;
- (void)scrollPageDown:(id)sender;
- (void)scrollLineUp:(id)sender;
- (void)scrollLineDown:(id)sender;

- (void)scrollToBeginningOfDocument:(id)sender;
- (void)scrollToEndOfDocument:(id)sender;



- (void)transpose:(id)sender;
- (void)transposeWords:(id)sender;



- (void)selectAll:(id)sender;
- (void)selectParagraph:(id)sender;
- (void)selectLine:(id)sender;
- (void)selectWord:(id)sender;



- (void)indent:(id)sender;
- (void)insertTab:(id)sender;
- (void)insertBacktab:(id)sender;
- (void)insertNewline:(id)sender;
- (void)insertParagraphSeparator:(id)sender;
- (void)insertNewlineIgnoringFieldEditor:(id)sender;
- (void)insertTabIgnoringFieldEditor:(id)sender;
- (void)insertLineBreak:(id)sender;
- (void)insertContainerBreak:(id)sender;
- (void)insertSingleQuoteIgnoringSubstitution:(id)sender;
- (void)insertDoubleQuoteIgnoringSubstitution:(id)sender;



- (void)changeCaseOfLetter:(id)sender;
- (void)uppercaseWord:(id)sender;
- (void)lowercaseWord:(id)sender;
- (void)capitalizeWord:(id)sender;



- (void)deleteForward:(id)sender;
- (void)deleteBackward:(id)sender;
- (void)deleteBackwardByDecomposingPreviousCharacter:(id)sender;
- (void)deleteWordForward:(id)sender;
- (void)deleteWordBackward:(id)sender;
- (void)deleteToBeginningOfLine:(id)sender;
- (void)deleteToEndOfLine:(id)sender;
- (void)deleteToBeginningOfParagraph:(id)sender;
- (void)deleteToEndOfParagraph:(id)sender;

- (void)yank:(id)sender;



- (void)complete:(id)sender;



- (void)setMark:(id)sender;
- (void)deleteToMark:(id)sender;
- (void)selectToMark:(id)sender;
- (void)swapWithMark:(id)sender;



- (void)cancelOperation:(id)sender;



- (void)makeBaseWritingDirectionNatural:(id)sender;
- (void)makeBaseWritingDirectionLeftToRight:(id)sender;
- (void)makeBaseWritingDirectionRightToLeft:(id)sender;

- (void)makeTextWritingDirectionNatural:(id)sender;
- (void)makeTextWritingDirectionLeftToRight:(id)sender;
- (void)makeTextWritingDirectionRightToLeft:(id)sender;




- (void)quickLookPreviewItems:(id)sender;
@end

@interface NSResponder(NSUndoSupport)
- (NSUndoManager *)undoManager;
@end

@interface NSResponder (NSControlEditingSupport)



- (BOOL)validateProposedFirstResponder:(NSResponder *)responder forEvent:(NSEvent *)event;

@end



@interface NSResponder(NSErrorPresentation)
- (void)presentError:(NSError *)error modalForWindow:(NSWindow *)window delegate:(id)delegate didPresentSelector:(SEL)didPresentSelector contextInfo:(void *)contextInfo;







- (BOOL)presentError:(NSError *)error;





- (NSError *)willPresentError:(NSError *)error;

@end


@interface NSResponder(NSTextFinderSupport)

- (void)performTextFinderAction:(id)sender;

@end


@interface NSResponder(NSDeprecated)



- (BOOL)performMnemonic:(NSString *)theString;

@end
