#import <AppKit/NSController.h>


@class NSDictionary, NSUserDefaults;

@interface NSUserDefaultsController : NSController {
@private
 void *_reserved3;
 void *_reserved4;
    NSUserDefaults *_defaults;
    NSMutableDictionary *_valueBuffer;
    NSDictionary *_initialValues;
    struct __userDefaultsControllerFlags {
        unsigned int _sharedInstance:1;
        unsigned int _appliesImmediately:1;
        unsigned int _reservedUserDefaultsController:30;
    } _userDefaultsControllerFlags;
}

+ (id)sharedUserDefaultsController;

- (id)initWithDefaults:(NSUserDefaults *)defaults initialValues:(NSDictionary *)initialValues;

- (NSUserDefaults *)defaults;
- (void)setInitialValues:(NSDictionary *)initialValues;
- (NSDictionary *)initialValues;

- (void)setAppliesImmediately:(BOOL)flag;
- (BOOL)appliesImmediately;

- (BOOL)hasUnappliedChanges;

- (id)values;

- (void)revert:(id)sender;
- (void)save:(id)sender;
- (void)revertToInitialValues:(id)sender;

@end
