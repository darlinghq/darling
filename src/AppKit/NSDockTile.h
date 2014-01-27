#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>

#define NSAppKitVersionNumberWithDockTilePlugInSupport 1001.0


@class NSView;


@interface NSDockTile : NSObject {
    @private;
    id _owner;
    void *_dockContextRef;
    NSView *_contentView;
    NSView *_frameView;
    NSView *_backstopView;
    NSString *_badgeLabel;
    struct {
        unsigned int showsAppBadge:1;
        unsigned int reserved:31;
    } _dFlags;
    NSSize _dockTileSize;
    id _miniViewController;
    id reserved[4];
}



- (NSSize)size;



- (void)setContentView:(NSView *)view;
- (NSView *)contentView;



- (void)display;



- (void)setShowsApplicationBadge:(BOOL)flag;
- (BOOL)showsApplicationBadge;



- (void)setBadgeLabel:(NSString *)string;
- (NSString *)badgeLabel;



- (id)owner;

@end



@class NSMenu;



@protocol NSDockTilePlugIn <NSObject>
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6

@required
- (void)setDockTile:(NSDockTile*)dockTile;

@optional
- (NSMenu*)dockMenu;
#endif
@end
