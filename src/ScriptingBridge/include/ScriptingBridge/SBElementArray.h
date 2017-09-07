#import <Foundation/Foundation.h>
#import <ScriptingBridge/SBObject.h>

NS_ASSUME_NONNULL_BEGIN

NS_CLASS_AVAILABLE(10_5, NA)
@interface SBElementArray<ObjectType> : NSMutableArray<ObjectType>
{
	SBObject *_parent;
	DescType _elementCode;
	void *_reserved;
}

- (ObjectType) objectWithName:(NSString *)name;
- (ObjectType) objectWithID:(id)identifier;
- (ObjectType) objectAtLocation:(id)location;

- (NSArray<id> *) arrayByApplyingSelector:(SEL)selector;
- (NSArray<id> *) arrayByApplyingSelector:(SEL)aSelector withObject:(id)argument;

- (nullable NSArray<ObjectType> *) get;

@end

NS_ASSUME_NONNULL_END
