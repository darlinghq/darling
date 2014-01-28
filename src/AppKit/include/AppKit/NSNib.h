#import <AppKit/AppKitDefines.h>
#import <Foundation/Foundation.h>


@class NSArray, NSBundle, NSData, NSDictionary, NSIBObjectData, NSURL;

@interface NSNib : NSObject <NSCoding> {
   @private
    NSData *_data;
    NSArray *_images;
    NSArray *_sounds;
    NSBundle *_bundle;
    struct _NSNibFlags {
        unsigned int _isKeyed:1;
        unsigned int _inheritsDecodeTimeBundle:1;
        unsigned int _reserved:30;
    } _flags;
    NSString *_path;
    id reserved2;
}







- (id)initWithNibNamed:(NSString *)nibName bundle:(NSBundle *)bundle;




- (id)initWithNibData:(NSData *)nibData bundle:(NSBundle *)bundle;
- (BOOL)instantiateWithOwner:(id)owner topLevelObjects:(NSArray **)topLevelObjects;

@end

@interface NSNib (NSDeprecated)
- (id)initWithContentsOfURL:(NSURL *)nibFileURL;
- (BOOL)instantiateNibWithExternalNameTable:(NSDictionary *)externalNameTable;
- (BOOL)instantiateNibWithOwner:(id)owner topLevelObjects:(NSArray **)topLevelObjects;
@end


EXTERN_C NSString *NSNibOwner;
EXTERN_C NSString *NSNibTopLevelObjects;
