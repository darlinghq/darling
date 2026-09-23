#import <Foundation/Foundation.h>

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
@end
