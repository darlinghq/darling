#import <AppKit/NSToolbarItem.h>
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>





@interface NSToolbarItemGroup : NSToolbarItem {
    @private
    id _groupItems;
    struct {
      unsigned dirtiedLayout:1;
      unsigned reserved:31;
    } _giFlags;
    id _giReserved;
}



- (void)setSubitems:(NSArray *)subitems;
- (NSArray *)subitems;

@end
