#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

@class NSString, NSArray;


@interface NSTextAlternatives : NSObject {
@private
    NSString *_primaryString;
    NSArray *_alternativeStrings;
    id _internal;
}

- (id)initWithPrimaryString:(NSString *)primaryString alternativeStrings:(NSArray *)alternativeStrings;

@property (readonly) NSString *primaryString;
@property (readonly) NSArray *alternativeStrings;

- (void)noteSelectedAlternativeString:(NSString *)alternativeString;

@end

EXTERN_C NSString *NSTextAlternativesSelectedAlternativeStringNotification;
