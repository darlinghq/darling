#ifndef NSAPPLEEVENTDESCRIPTOR_H
#define NSAPPLEEVENTDESCRIPTOR_H
#include <Foundation/NSObject.h>
#include <Foundation/NSData.h>
#include <Foundation/NSString.h>
#include <CoreServices/MacTypes.h>

typedef int AEEventClass;
typedef int AEEventID;
typedef int AEReturnID;
typedef UInt32 AEKeyword;
typedef int AETransactionID;
typedef int DescType;

struct AEDesc;

@interface NSAppleEventDescriptor : NSObject

+ (NSAppleEventDescriptor *)appleEventWithEventClass:(AEEventClass)eventClass
                                             eventID:(AEEventID)eventID
                                    targetDescriptor:(NSAppleEventDescriptor *)addressDescriptor
                                            returnID:(AEReturnID)returnID
                                       transactionID:(AETransactionID)transactionID;

+ (NSAppleEventDescriptor *)descriptorWithBoolean:(Boolean)boolean;
+ (NSAppleEventDescriptor *)descriptorWithDescriptorType:(DescType)descriptorType
                                                   bytes:(const void *)bytes
                                                  length:(NSUInteger)byteCount;

+ (NSAppleEventDescriptor *)descriptorWithDescriptorType:(DescType)descriptorType
                                                    data:(NSData *)data;

+ (NSAppleEventDescriptor *)descriptorWithEnumCode:(OSType)enumerator;
+ (NSAppleEventDescriptor *)descriptorWithInt32:(SInt32)signedInt;
+ (NSAppleEventDescriptor *)descriptorWithString:(NSString *)string;
+ (NSAppleEventDescriptor *)descriptorWithTypeCode:(OSType)typeCode;
+ (NSAppleEventDescriptor *)listDescriptor;
+ (NSAppleEventDescriptor *)nullDescriptor;
+ (NSAppleEventDescriptor *)recordDescriptor;

- (id)initListDescriptor;
- (id)initRecordDescriptor;
- (id)initWithAEDescNoCopy:(const AEDesc *)aeDesc;
- (id)initWithDescriptorType:(DescType)descriptorType
                       bytes:(const void *)bytes
                      length:(NSUInteger)byteCount;
- (id)initWithDescriptorType:(DescType)descriptorType
                        data:(NSData *)data;
- (id)initWithEventClass:(AEEventClass)eventClass
                 eventID:(AEEventID)eventID
        targetDescriptor:(NSAppleEventDescriptor *)addressDescriptor
                returnID:(AEReturnID)returnID
           transactionID:(AETransactionID)transactionID;

- (const AEDesc *)aeDesc;
- (NSAppleEventDescriptor *)attributeDescriptorForKeyword:(AEKeyword)keyword;

- (Boolean)booleanValue;
- (SInt32)int32Value;
- (NSString *)stringValue;

- (DescType)descriptorType;
- (OSType)enumCodeValue;
- (AEEventClass)eventClass;
- (AEEventID)eventID;
- (AETransactionID)transactionID;
- (OSType)typeCodeValue;

- (NSAppleEventDescriptor *)coerceToDescriptorType:(DescType)descriptorType;
- (NSData *)data;
- (NSAppleEventDescriptor *)descriptorAtIndex:(NSInteger)anIndex;
- (NSAppleEventDescriptor *)descriptorForKeyword:(AEKeyword)keyword;

- (void)insertDescriptor:(NSAppleEventDescriptor *)descriptor
                 atIndex:(NSInteger)anIndex;
- (void)removeDescriptorAtIndex:(NSInteger)anIndex;
- (void)removeDescriptorWithKeyword:(AEKeyword)keyword;
- (void)removeParamDescriptorWithKeyword:(AEKeyword)keyword;

- (AEKeyword)keywordForDescriptorAtIndex:(NSInteger)anIndex;
- (NSInteger)numberOfItems;
- (NSAppleEventDescriptor *)paramDescriptorForKeyword:(AEKeyword)keyword;
- (AEReturnID)returnID;

- (void)setAttributeDescriptor:(NSAppleEventDescriptor *)descriptor
                    forKeyword:(AEKeyword)keyword;
- (void)setDescriptor:(NSAppleEventDescriptor *)descriptor
           forKeyword:(AEKeyword)keyword;
- (void)setParamDescriptor:(NSAppleEventDescriptor *)descriptor
                forKeyword:(AEKeyword)keyword;

@end

#endif
