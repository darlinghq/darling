#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitDefines.h>

@class NSArray, NSString;
@protocol NSPasteboardWriting;


EXTERN_C NSString * const NSDraggingImageComponentIconKey;
EXTERN_C NSString * const NSDraggingImageComponentLabelKey;


@interface NSDraggingImageComponent : NSObject {
@private
    NSString *_key;
    id _contents;
    NSRect _frame;
}

+ (id)draggingImageComponentWithKey:(NSString *)key;


- (id)initWithKey:(NSString *)key;



@property(copy) NSString *key;



@property(retain) id contents;



@property NSRect frame;

@end


@interface NSDraggingItem : NSObject {
@private
    id _item;
    NSRect _frame;
    NSArray *_components;
    NSArray *(^_componentsProvider)(void);
    NSInteger _reserved1;
    NSInteger _reserved2;
}



- (id)initWithPasteboardWriter:(id <NSPasteboardWriting>) pasteboardWriter;



@property(retain, readonly) id item;



@property NSRect draggingFrame;

#if NS_BLOCKS_AVAILABLE


@property(copy) NSArray*(^imageComponentsProvider)(void);
#endif



- (void)setDraggingFrame:(NSRect)frame contents:(id)contents;



@property(copy, readonly) NSArray *imageComponents;

@end
