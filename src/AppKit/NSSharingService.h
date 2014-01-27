#import <AppKit/AppKitDefines.h>
#import <AppKit/NSPasteboard.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>
@class NSString, NSImage, NSArray, NSView, NSError, NSWindow;






EXTERN_C NSString * const NSSharingServiceNamePostOnFacebook;
EXTERN_C NSString * const NSSharingServiceNamePostOnTwitter;
EXTERN_C NSString * const NSSharingServiceNamePostOnSinaWeibo;
EXTERN_C NSString * const NSSharingServiceNameComposeEmail;
EXTERN_C NSString * const NSSharingServiceNameComposeMessage;
EXTERN_C NSString * const NSSharingServiceNameSendViaAirDrop;
EXTERN_C NSString * const NSSharingServiceNameAddToSafariReadingList;
EXTERN_C NSString * const NSSharingServiceNameAddToIPhoto;
EXTERN_C NSString * const NSSharingServiceNameAddToAperture;
EXTERN_C NSString * const NSSharingServiceNameUseAsTwitterProfileImage;
EXTERN_C NSString * const NSSharingServiceNameUseAsDesktopPicture;
EXTERN_C NSString * const NSSharingServiceNamePostImageOnFlickr;
EXTERN_C NSString * const NSSharingServiceNamePostVideoOnVimeo;
EXTERN_C NSString * const NSSharingServiceNamePostVideoOnYouku;
EXTERN_C NSString * const NSSharingServiceNamePostVideoOnTudou;


@protocol NSSharingServiceDelegate;


@interface NSSharingService : NSObject {
@private
    id _reserved;
}
@property (assign) id <NSSharingServiceDelegate> delegate;
@property (readonly, copy) NSString *title;
@property (readonly, retain) NSImage *image;
@property (readonly, retain) NSImage *alternateImage;




+ (NSArray *)sharingServicesForItems:(NSArray *)items;




+ (NSSharingService *)sharingServiceNamed:(NSString *)serviceName;


#if NS_BLOCKS_AVAILABLE


- (id)initWithTitle:(NSString *)title image:(NSImage *)image alternateImage:(NSImage *)alternateImage handler:(void (^)(void))block;
#endif




- (BOOL)canPerformWithItems:(NSArray *)items;




- (void)performWithItems:(NSArray *)items;

@end
typedef NS_ENUM(NSInteger, NSSharingContentScope) {
    NSSharingContentScopeItem,
    NSSharingContentScopePartial,
    NSSharingContentScopeFull
} NS_ENUM_AVAILABLE_MAC(10_8);

@protocol NSSharingServiceDelegate <NSObject>
@optional
- (void)sharingService:(NSSharingService *)sharingService willShareItems:(NSArray *)items;
- (void)sharingService:(NSSharingService *)sharingService didFailToShareItems:(NSArray *)items error:(NSError *)error;
- (void)sharingService:(NSSharingService *)sharingService didShareItems:(NSArray *)items;




- (NSRect)sharingService:(NSSharingService *)sharingService sourceFrameOnScreenForShareItem:(id <NSPasteboardWriting>)item;
- (NSImage *)sharingService:(NSSharingService *)sharingService transitionImageForShareItem:(id <NSPasteboardWriting>)item contentRect:(NSRect *)contentRect;
- (NSWindow *)sharingService:(NSSharingService *)sharingService sourceWindowForShareItems:(NSArray *)items sharingContentScope:(NSSharingContentScope *)sharingContentScope;

@end





@protocol NSSharingServicePickerDelegate;


@interface NSSharingServicePicker : NSObject
{
@private
    id _reserved;
}

@property (assign) id <NSSharingServicePickerDelegate> delegate;



- (id)initWithItems:(NSArray *)items;




- (void)showRelativeToRect:(NSRect)rect ofView:(NSView *)view preferredEdge:(NSRectEdge)preferredEdge;

@end


@protocol NSSharingServicePickerDelegate <NSObject>
@optional
- (NSArray *)sharingServicePicker:(NSSharingServicePicker *)sharingServicePicker sharingServicesForItems:(NSArray *)items proposedSharingServices:(NSArray *)proposedServices;




- (id <NSSharingServiceDelegate>)sharingServicePicker:(NSSharingServicePicker *)sharingServicePicker delegateForSharingService:(NSSharingService *)sharingService;




- (void)sharingServicePicker:(NSSharingServicePicker *)sharingServicePicker didChooseSharingService:(NSSharingService *)service;

@end
