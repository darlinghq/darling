#import <Foundation/Foundation.h>
#import <ApplicationServices/ApplicationServices.h>

NS_ASSUME_NONNULL_BEGIN

@class SBAppContext, SBElementArray;

NS_CLASS_AVAILABLE(10_5, NA)
@interface SBObject : NSObject <NSCoding>
{
	AEDesc _specifier;
	SBAppContext *_ctx;
	id _reserved;
}

- (instancetype) init;
- (instancetype) initWithProperties:(NSDictionary *)properties;
- (instancetype) initWithData:(id)data;

- (nullable id) get;

- (nullable NSError *) lastError NS_AVAILABLE(10_6, NA);

@end

@interface SBObject (SBGlueInterface)

- (instancetype) initWithElementCode:(DescType)code properties:(nullable NSDictionary<NSString *, id> *)properties data:(nullable id)data;

- (SBObject *) propertyWithCode:(AEKeyword)code;
- (SBObject *) propertyWithClass:(Class)cls code:(AEKeyword)code;

- (SBElementArray *) elementArrayWithCode:(DescType)code;

- (id) sendEvent:(AEEventClass)eventClass id:(AEEventID)eventID parameters:(DescType)firstParamCode, ...;

- (void) setTo:(nullable id)value;

@end

NS_ASSUME_NONNULL_END
