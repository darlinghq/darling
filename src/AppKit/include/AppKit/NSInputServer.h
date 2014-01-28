#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSRange.h>



@protocol NSInputServiceProvider

- (void) insertText:(id)aString client:(id)sender;
- (void) doCommandBySelector:(SEL)aSelector client:(id)sender;
- (void) markedTextAbandoned:(id)sender;
- (void) markedTextSelectionChanged:(NSRange)newSel client:(id)sender;
- (void) terminate:(id)sender;



- (BOOL) canBeDisabled;



- (BOOL) wantsToInterpretAllKeystrokes;


- (BOOL) wantsToHandleMouseEvents;



- (BOOL) wantsToDelayTextChangeNotifications;



- (void) inputClientBecomeActive:(id)sender;
- (void) inputClientResignActive:(id)sender;



- (void) inputClientEnabled:(id)sender;
- (void) inputClientDisabled:(id)sender;



- (void) activeConversationWillChange:(id)sender fromOldConversation:(NSInteger)oldConversation;
- (void) activeConversationChanged:(id)sender toNewConversation:(NSInteger)newConversation;

@end



@protocol NSInputServerMouseTracker
- (BOOL) mouseDownOnCharacterIndex:(NSUInteger)theIndex atCoordinate:(NSPoint)thePoint withModifier:(NSUInteger)theFlags client:(id)sender;
- (BOOL) mouseDraggedOnCharacterIndex:(NSUInteger)theIndex atCoordinate:(NSPoint)thePoint withModifier:(NSUInteger)theFlags client:(id)sender;
- (void) mouseUpOnCharacterIndex:(NSUInteger)theIndex atCoordinate:(NSPoint)thePoint withModifier:(NSUInteger)theFlags client:(id)sender;
@end

@interface NSInputServer : NSObject <NSInputServiceProvider, NSInputServerMouseTracker> {
    NSString *_name;
    id _delegate;
}

- initWithDelegate:(id)aDelegate name:(NSString *)name;
@end
