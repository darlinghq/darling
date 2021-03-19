#import <Foundation/NSDictionary.h>

@interface NSDictionary<KeyType, ObjectType> (BetterAccessors)

- (ObjectType)stringForKey: (KeyType)key;

@end
