#import <Foundation/Foundation.h>

// Message type numbers whose names the framework reports. Types 0, 1 and anything above 5 are
// all named "default".
typedef NS_ENUM(NSInteger, CSKMessageType) {
	CSKMessageTypeInfo = 2,
	CSKMessageTypeDebug = 3,
	CSKMessageTypeError = 4,
	CSKMessageTypeFault = 5,
};

// One log entry, whatever store it came from.
@interface CSKMessage : NSObject
@property (readonly, copy) NSDate *date;
@property (readonly, copy) NSString *sender;
@property (readonly) pid_t processID;
@property (readonly, copy) NSString *subsystem;
@property (readonly, copy) NSString *category;
@property (readonly) int level;
@property (readonly, copy) NSString *composedMessage;
- (instancetype)initWithDate:(NSDate *)date sender:(NSString *)sender processID:(pid_t)pid subsystem:(NSString *)subsystem category:(NSString *)category level:(int)level message:(NSString *)message;
+ (NSString *)localizedMessageTypeNameForType:(CSKMessageType)type;
@end
