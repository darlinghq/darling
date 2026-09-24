#import <Foundation/Foundation.h>

// Console's filter bar asks for the names of types 4 and 5 when it builds its Errors and Faults
// filter, so these are taken to be those two types.
typedef NS_ENUM(NSInteger, CSKMessageType) {
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
