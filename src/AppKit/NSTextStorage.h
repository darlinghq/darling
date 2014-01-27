#import <Foundation/NSObject.h>
#import <AppKit/NSAttributedString.h>

@class NSLayoutManager;
@class NSMutableArray;
@protocol NSTextStorageDelegate;



enum {
    NSTextStorageEditedAttributes = 1,
    NSTextStorageEditedCharacters = 2
};

@interface NSTextStorage : NSMutableAttributedString {

    NSRange _editedRange;
    NSInteger _editedDelta;
    struct {
        unsigned int editedMask:8;
        unsigned int :8;
        unsigned int disabled:16;
#if __LP64__
        unsigned int :32;
#endif
    } _flags;
    NSMutableArray *_layoutManagers;
    id _sideData;
}



- (void)addLayoutManager:(NSLayoutManager *)obj;
- (void)removeLayoutManager:(NSLayoutManager *)obj;
- (NSArray *)layoutManagers;



- (void)edited:(NSUInteger)editedMask range:(NSRange)range changeInLength:(NSInteger)delta;



- (void)processEditing;


- (void)invalidateAttributesInRange:(NSRange)range;


- (void)ensureAttributesAreFixedInRange:(NSRange)range;



- (BOOL)fixesAttributesLazily;



- (NSUInteger)editedMask;
- (NSRange)editedRange;
- (NSInteger)changeInLength;



- (void)setDelegate:(id <NSTextStorageDelegate>)delegate;
- (id <NSTextStorageDelegate>)delegate;

@end




@protocol NSTextStorageDelegate <NSObject>
@optional



- (void)textStorageWillProcessEditing:(NSNotification *)notification;
- (void)textStorageDidProcessEditing:(NSNotification *)notification;

@end



EXTERN_C NSString *NSTextStorageWillProcessEditingNotification;
EXTERN_C NSString *NSTextStorageDidProcessEditingNotification;
