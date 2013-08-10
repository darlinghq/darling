#include "NSAppleEventDescriptor.h"
#include <util/debug.h>

@implementation NSAppleEventDescriptor


+ (NSAppleEventDescriptor *)appleEventWithEventClass:(AEEventClass)eventClass
                                             eventID:(AEEventID)eventID
                                    targetDescriptor:(NSAppleEventDescriptor *)addressDescriptor
                                            returnID:(AEReturnID)returnID
                                       transactionID:(AETransactionID)transactionID
{
	return [[[self alloc] initWithEventClass: eventClass
	                                 eventID: eventID
	                        targetDescriptor: addressDescriptor
	                                returnID: returnID
	                           transactionID: transactionID] autorelease];
}

+ (NSAppleEventDescriptor *)descriptorWithBoolean:(Boolean)boolean
{
	STUB();
	return [[self alloc] init];
}

+ (NSAppleEventDescriptor *)descriptorWithDescriptorType:(DescType)descriptorType
                                                   bytes:(const void *)bytes
                                                  length:(NSUInteger)byteCount
{
	return [[[self alloc] initWithDescriptorType: descriptorType
	                                       bytes: bytes
	                                      length: byteCount] autorelease];
}

+ (NSAppleEventDescriptor *)descriptorWithDescriptorType:(DescType)descriptorType
                                                    data:(NSData *)data
{
	return [[[self alloc] initWithDescriptorType: descriptorType
	                                        data: data] autorelease];
}

+ (NSAppleEventDescriptor *)descriptorWithEnumCode:(OSType)enumerator
{
	STUB();
	return [[self alloc] init];
}

+ (NSAppleEventDescriptor *)descriptorWithInt32:(SInt32)signedInt
{
	STUB();
	return [[self alloc] init];
}

+ (NSAppleEventDescriptor *)descriptorWithString:(NSString *)string
{
	STUB();
	return [[self alloc] init];
}

+ (NSAppleEventDescriptor *)descriptorWithTypeCode:(OSType)typeCode
{
	STUB();
	return [[self alloc] init];
}

+ (NSAppleEventDescriptor *)listDescriptor
{
	return [[[self alloc] initListDescriptor] autorelease];
}

+ (NSAppleEventDescriptor *)nullDescriptor
{
	STUB();
	return [[self alloc] init];
}

+ (NSAppleEventDescriptor *)recordDescriptor
{
	return [[[self alloc] initRecordDescriptor] autorelease];
}

- (id)initListDescriptor
{
	STUB();
	return [self init];
}

- (id)initRecordDescriptor
{
	STUB();
	return [self init];
}

- (id)initWithAEDescNoCopy:(const AEDesc *)aeDesc
{
	STUB();
	return [self init];
}

- (id)initWithDescriptorType:(DescType)descriptorType
                       bytes:(const void *)bytes
                      length:(NSUInteger)byteCount
{
	STUB();
	return [self init];
}

- (id)initWithDescriptorType:(DescType)descriptorType
                        data:(NSData *)data
{
	STUB();
	return [self init];
}

- (id)initWithEventClass:(AEEventClass)eventClass
                 eventID:(AEEventID)eventID
        targetDescriptor:(NSAppleEventDescriptor *)addressDescriptor
                returnID:(AEReturnID)returnID
           transactionID:(AETransactionID)transactionID
{
	STUB();
	return [self init];
}

- (const AEDesc *)aeDesc
{
	STUB();
	return NULL;
}

- (NSAppleEventDescriptor *)attributeDescriptorForKeyword:(AEKeyword)keyword
{
	STUB();
	return NULL;
}

- (Boolean)booleanValue
{
	STUB();
	return false;
}

- (SInt32)int32Value
{
	STUB();
	return 0;
}

- (NSString *)stringValue
{
	STUB();
	return NULL;
}

- (DescType)descriptorType
{
	STUB();
	return 0;
}

- (OSType)enumCodeValue
{
	STUB();
	return 0;
}

- (AEEventClass)eventClass
{
	STUB();
	return 0;
}

- (AEEventID)eventID
{
	STUB();
	return 0;
}

- (AETransactionID)transactionID
{
	STUB();
	return 0;
}

- (OSType)typeCodeValue
{
	STUB();
	return 0;
}

- (NSAppleEventDescriptor *)coerceToDescriptorType:(DescType)descriptorType
{
	STUB();
	return NULL;
}

- (NSData *)data
{
	STUB();
	return NULL;
}

- (NSAppleEventDescriptor *)descriptorAtIndex:(NSInteger)anIndex
{
	STUB();
	return NULL;
}

- (NSAppleEventDescriptor *)descriptorForKeyword:(AEKeyword)keyword
{
	STUB();
	return NULL;
}


- (void)insertDescriptor:(NSAppleEventDescriptor *)descriptor
                 atIndex:(NSInteger)anIndex
{
	STUB();
}

- (void)removeDescriptorAtIndex:(NSInteger)anIndex
{
	STUB();
}

- (void)removeDescriptorWithKeyword:(AEKeyword)keyword
{
	STUB();
}

- (void)removeParamDescriptorWithKeyword:(AEKeyword)keyword
{
	STUB();
}


- (AEKeyword)keywordForDescriptorAtIndex:(NSInteger)anIndex
{
	STUB();
	return 0;
}

- (NSInteger)numberOfItems
{
	STUB();
	return 0;
}

- (NSAppleEventDescriptor *)paramDescriptorForKeyword:(AEKeyword)keyword
{
	STUB();
	return 0;
}

- (AEReturnID)returnID
{
	STUB();
	return 0;
}


- (void)setAttributeDescriptor:(NSAppleEventDescriptor *)descriptor
                    forKeyword:(AEKeyword)keyword
{
	STUB();
}

- (void)setDescriptor:(NSAppleEventDescriptor *)descriptor
           forKeyword:(AEKeyword)keyword
           
{
	STUB();
}

- (void)setParamDescriptor:(NSAppleEventDescriptor *)descriptor
                forKeyword:(AEKeyword)keyword
{
	STUB();
}



@end
