/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#import <CoreML/CoreML.h>
#include <dispatch/dispatch.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdlib.h>
#include <string.h>

// The classes keep their state in synthesized instance variables, which the
// legacy 32-bit Objective-C ABI does not support, and the framework exists
// only for 64-bit processes on macOS. The 32-bit slice is therefore empty.
#ifdef __LP64__

NSString *const MLModelErrorDomain = @"com.apple.CoreML";

MLModelMetadataKey const MLModelDescriptionKey = @"MLModelDescriptionKey";
MLModelMetadataKey const MLModelVersionStringKey = @"MLModelVersionStringKey";
MLModelMetadataKey const MLModelAuthorKey = @"MLModelAuthorKey";
MLModelMetadataKey const MLModelLicenseKey = @"MLModelLicenseKey";
MLModelMetadataKey const MLModelCreatorDefinedKey = @"MLModelCreatorDefinedKey";

MLFeatureValueImageOption const MLFeatureValueImageOptionCropRect = @"MLFeatureValueImageOptionCropRect";
MLFeatureValueImageOption const MLFeatureValueImageOptionCropAndScale = @"MLFeatureValueImageOptionCropAndScale";

static BOOL failWithError(NSError **error, MLModelError code, NSString *description)
{
	if (error != NULL) {
		*error = [NSError errorWithDomain:MLModelErrorDomain
		                             code:code
		                         userInfo:@{ NSLocalizedDescriptionKey: description }];
	}
	return NO;
}

static NSString *const kUnavailableDescription = @"Core ML is not available in Darling: models cannot be loaded";

#pragma mark Half precision

static float halfToFloat(uint16_t h)
{
	uint32_t sign = (uint32_t) (h & 0x8000) << 16;
	uint32_t exponent = (h >> 10) & 0x1f;
	uint32_t mantissa = h & 0x3ff;
	uint32_t bits;

	if (exponent == 0) {
		if (mantissa == 0) {
			bits = sign;
		} else {
			// Subnormal half: normalise it.
			exponent = 127 - 15 + 1;
			while ((mantissa & 0x400) == 0) {
				mantissa <<= 1;
				exponent--;
			}
			bits = sign | (exponent << 23) | ((mantissa & 0x3ff) << 13);
		}
	} else if (exponent == 0x1f) {
		bits = sign | 0x7f800000 | (mantissa << 13);
	} else {
		bits = sign | ((exponent + 127 - 15) << 23) | (mantissa << 13);
	}

	float f;
	memcpy(&f, &bits, sizeof(f));
	return f;
}

static uint16_t floatToHalf(float f)
{
	uint32_t bits;
	memcpy(&bits, &f, sizeof(bits));

	uint16_t sign = (bits >> 16) & 0x8000;
	uint32_t abs = bits & 0x7fffffff;

	if (abs >= 0x7f800000)
		return sign | 0x7c00 | (abs > 0x7f800000 ? 0x200 : 0); // infinity or NaN
	if (abs >= 0x477ff000)
		return sign | 0x7c00; // rounds to infinity
	if (abs < 0x33000001)
		return sign; // rounds to zero

	int exponent = (int) (abs >> 23) - 127;
	uint32_t mantissa = (abs & 0x7fffff) | 0x800000;

	if (exponent < -14) {
		// Subnormal half, round to nearest even.
		int shift = -14 - exponent + 13;
		uint32_t value = mantissa >> shift;
		uint32_t rest = mantissa & ((1u << shift) - 1);
		uint32_t halfway = 1u << (shift - 1);
		if (rest > halfway || (rest == halfway && (value & 1)))
			value++;
		return sign | value;
	}

	uint32_t value = ((uint32_t) (exponent + 15) << 10) | ((mantissa >> 13) & 0x3ff);
	uint32_t rest = mantissa & 0x1fff;
	if (rest > 0x1000 || (rest == 0x1000 && (value & 1)))
		value++;
	return sign | value;
}

#pragma mark MLMultiArray

static size_t elementSize(MLMultiArrayDataType type)
{
	return (size_t) (type & 0xffff) / 8;
}

static BOOL validType(MLMultiArrayDataType type)
{
	switch (type) {
		case MLMultiArrayDataTypeDouble:
		case MLMultiArrayDataTypeFloat32:
		case MLMultiArrayDataTypeFloat16:
		case MLMultiArrayDataTypeInt32:
		case MLMultiArrayDataTypeInt8:
			return YES;
		default:
			return NO;
	}
}

@implementation MLMultiArray {
	void *_data;
	void (^_deallocator)(void *);
	size_t _byteLength;
}

- (instancetype)initWithDataPointer:(void *)dataPointer
                              shape:(NSArray<NSNumber *> *)shape
                           dataType:(MLMultiArrayDataType)dataType
                            strides:(NSArray<NSNumber *> *)strides
                        deallocator:(void (^)(void *))deallocator
                              error:(NSError **)error
{
	NSUInteger rank = [shape count];
	BOOL valid = validType(dataType) && [strides count] == rank && dataPointer != NULL;

	NSInteger count = 1;
	NSInteger extent = 1;
	for (NSUInteger i = 0; valid && i < rank; i++) {
		NSInteger dim = [shape[i] integerValue];
		NSInteger stride = [strides[i] integerValue];
		if (dim < 0 || stride <= 0) {
			valid = NO;
			break;
		}
		count *= dim;
		extent += (dim > 0 ? dim - 1 : 0) * stride;
	}

	if (!valid) {
		failWithError(error, MLModelErrorGeneric, @"Invalid multi-array shape, strides or data type");
		[self release];
		return nil;
	}

	if ((self = [super init]) != nil) {
		_data = dataPointer;
		_deallocator = [deallocator copy];
		_dataType = dataType;
		_shape = [shape copy];
		_strides = [strides copy];
		_count = count;
		_byteLength = (size_t) extent * elementSize(dataType);
	}
	return self;
}

- (instancetype)initWithShape:(NSArray<NSNumber *> *)shape dataType:(MLMultiArrayDataType)dataType error:(NSError **)error
{
	NSUInteger rank = [shape count];
	NSMutableArray *strides = [NSMutableArray arrayWithCapacity:rank];
	NSInteger stride = 1;
	size_t total = 1;
	BOOL valid = validType(dataType);

	for (NSUInteger i = rank; valid && i > 0; i--) {
		NSInteger dim = [shape[i - 1] integerValue];
		if (dim < 0) {
			valid = NO;
			break;
		}
		[strides insertObject:@(stride) atIndex:0];
		stride *= dim;
		total *= (size_t) dim;
	}

	// A shape with a zero dimension has no elements but still gets a buffer.
	void *data = valid ? calloc(total > 0 ? total : 1, elementSize(dataType)) : NULL;
	if (!valid || data == NULL) {
		free(data);
		failWithError(error, MLModelErrorGeneric, @"Invalid multi-array shape or data type");
		[self release];
		return nil;
	}

	return [self initWithDataPointer:data
	                           shape:shape
	                        dataType:dataType
	                         strides:strides
	                     deallocator:^(void *bytes) { free(bytes); }
	                           error:error];
}

- (void)dealloc
{
	if (_deallocator != nil) {
		_deallocator(_data);
		[_deallocator release];
	}
	[_shape release];
	[_strides release];
	[super dealloc];
}

- (void *)dataPointer
{
	return _data;
}

- (CVPixelBufferRef)pixelBuffer
{
	return NULL;
}

- (void)getBytesWithHandler:(void (NS_NOESCAPE ^)(const void *, NSInteger))handler
{
	handler(_data, (NSInteger) _byteLength);
}

- (void)getMutableBytesWithHandler:(void (NS_NOESCAPE ^)(void *, NSInteger, NSArray<NSNumber *> *))handler
{
	handler(_data, (NSInteger) _byteLength, _strides);
}

// Out of range indices raise NSInvalidArgumentException.
- (size_t)elementOffsetForPosition:(NSInteger)position
{
	if (position < 0 || position >= _count)
		[NSException raise:NSInvalidArgumentException format:@"MLMultiArray index %ld out of range", (long) position];

	size_t offset = 0;
	for (NSUInteger i = [_shape count]; i > 0; i--) {
		NSInteger dim = [_shape[i - 1] integerValue];
		offset += (size_t) (position % dim) * (size_t) [_strides[i - 1] integerValue];
		position /= dim;
	}
	return offset;
}

// Returns NO for a key with the wrong number of indices; out of range
// indices raise NSInvalidArgumentException.
- (BOOL)getElementOffset:(size_t *)offsetOut forKey:(NSArray<NSNumber *> *)key
{
	if ([key count] != [_shape count])
		return NO;

	size_t offset = 0;
	for (NSUInteger i = 0; i < [key count]; i++) {
		NSInteger index = [key[i] integerValue];
		if (index < 0 || index >= [_shape[i] integerValue])
			[NSException raise:NSInvalidArgumentException format:@"MLMultiArray index %ld out of range", (long) index];
		offset += (size_t) index * (size_t) [_strides[i] integerValue];
	}
	*offsetOut = offset;
	return YES;
}

- (NSNumber *)numberAtOffset:(size_t)offset
{
	switch (_dataType) {
		case MLMultiArrayDataTypeDouble:
			return @(((double *) _data)[offset]);
		case MLMultiArrayDataTypeFloat32:
			return @(((float *) _data)[offset]);
		case MLMultiArrayDataTypeFloat16:
			return @(halfToFloat(((uint16_t *) _data)[offset]));
		case MLMultiArrayDataTypeInt32:
			return @(((int32_t *) _data)[offset]);
		default:
			return @(((int8_t *) _data)[offset]);
	}
}

- (void)setNumber:(NSNumber *)number atOffset:(size_t)offset
{
	switch (_dataType) {
		case MLMultiArrayDataTypeDouble:
			((double *) _data)[offset] = [number doubleValue];
			break;
		case MLMultiArrayDataTypeFloat32:
			((float *) _data)[offset] = [number floatValue];
			break;
		case MLMultiArrayDataTypeFloat16:
			((uint16_t *) _data)[offset] = floatToHalf([number floatValue]);
			break;
		case MLMultiArrayDataTypeInt32:
			((int32_t *) _data)[offset] = (int32_t) [number intValue];
			break;
		default:
			((int8_t *) _data)[offset] = (int8_t) [number intValue];
			break;
	}
}

- (NSNumber *)objectAtIndexedSubscript:(NSInteger)idx
{
	return [self numberAtOffset:[self elementOffsetForPosition:idx]];
}

- (NSNumber *)objectForKeyedSubscript:(NSArray<NSNumber *> *)key
{
	size_t offset;
	return [self getElementOffset:&offset forKey:key] ? [self numberAtOffset:offset] : nil;
}

- (void)setObject:(NSNumber *)obj atIndexedSubscript:(NSInteger)idx
{
	[self setNumber:obj atOffset:[self elementOffsetForPosition:idx]];
}

- (void)setObject:(NSNumber *)obj forKeyedSubscript:(NSArray<NSNumber *> *)key
{
	size_t offset;
	if ([self getElementOffset:&offset forKey:key])
		[self setNumber:obj atOffset:offset];
}

@end

#pragma mark MLFeatureValue

@implementation MLFeatureValue {
	int64_t _int64Value;
	double _doubleValue;
	NSString *_stringValue;
	MLMultiArray *_multiArrayValue;
	NSDictionary *_dictionaryValue;
	CVPixelBufferRef _imageBufferValue;
}

- (instancetype)initWithType:(MLFeatureType)type undefined:(BOOL)undefined
{
	if ((self = [super init]) != nil) {
		_type = type;
		_undefined = undefined;
	}
	return self;
}

+ (instancetype)undefinedFeatureValueWithType:(MLFeatureType)type
{
	return [[[self alloc] initWithType:type undefined:YES] autorelease];
}

+ (instancetype)featureValueWithInt64:(int64_t)value
{
	MLFeatureValue *fv = [[[self alloc] initWithType:MLFeatureTypeInt64 undefined:NO] autorelease];
	fv->_int64Value = value;
	return fv;
}

+ (instancetype)featureValueWithDouble:(double)value
{
	MLFeatureValue *fv = [[[self alloc] initWithType:MLFeatureTypeDouble undefined:NO] autorelease];
	fv->_doubleValue = value;
	return fv;
}

+ (instancetype)featureValueWithString:(NSString *)value
{
	MLFeatureValue *fv = [[[self alloc] initWithType:MLFeatureTypeString undefined:NO] autorelease];
	fv->_stringValue = [value copy];
	return fv;
}

+ (instancetype)featureValueWithMultiArray:(MLMultiArray *)value
{
	MLFeatureValue *fv = [[[self alloc] initWithType:MLFeatureTypeMultiArray undefined:NO] autorelease];
	fv->_multiArrayValue = [value retain];
	return fv;
}

+ (instancetype)featureValueWithPixelBuffer:(CVPixelBufferRef)value
{
	MLFeatureValue *fv = [[[self alloc] initWithType:MLFeatureTypeImage undefined:NO] autorelease];
	fv->_imageBufferValue = value != NULL ? (CVPixelBufferRef) CFRetain(value) : NULL;
	return fv;
}

+ (instancetype)featureValueWithDictionary:(NSDictionary<id, NSNumber *> *)value error:(NSError **)error
{
	MLFeatureValue *fv = [[[self alloc] initWithType:MLFeatureTypeDictionary undefined:NO] autorelease];
	fv->_dictionaryValue = [value copy];
	return fv;
}

- (void)dealloc
{
	[_stringValue release];
	[_multiArrayValue release];
	[_dictionaryValue release];
	if (_imageBufferValue != NULL)
		CFRelease(_imageBufferValue);
	[super dealloc];
}

- (int64_t)int64Value
{
	return _int64Value;
}

- (double)doubleValue
{
	return _doubleValue;
}

- (NSString *)stringValue
{
	return _stringValue;
}

- (MLMultiArray *)multiArrayValue
{
	return _multiArrayValue;
}

- (NSDictionary *)dictionaryValue
{
	return _dictionaryValue;
}

- (CVPixelBufferRef)imageBufferValue
{
	return _imageBufferValue;
}

- (BOOL)isEqualToFeatureValue:(MLFeatureValue *)other
{
	if (other == self)
		return YES;
	if (other == nil || other.type != _type || other.undefined != _undefined)
		return NO;
	if (_undefined)
		return YES;

	switch (_type) {
		case MLFeatureTypeInt64:
			return other->_int64Value == _int64Value;
		case MLFeatureTypeDouble:
			return other->_doubleValue == _doubleValue;
		case MLFeatureTypeString:
			return [other->_stringValue isEqualToString:_stringValue];
		case MLFeatureTypeMultiArray:
			return other->_multiArrayValue == _multiArrayValue;
		case MLFeatureTypeDictionary:
			return [other->_dictionaryValue isEqualToDictionary:_dictionaryValue];
		case MLFeatureTypeImage:
			return other->_imageBufferValue == _imageBufferValue;
		default:
			return NO;
	}
}

- (BOOL)isEqual:(id)object
{
	return [object isKindOfClass:[MLFeatureValue class]] && [self isEqualToFeatureValue:object];
}

- (NSUInteger)hash
{
	return (NSUInteger) _type ^ (NSUInteger) _int64Value ^ [_stringValue hash] ^ [_dictionaryValue hash];
}

- (id)copyWithZone:(NSZone *)zone
{
	// Feature values are immutable.
	return [self retain];
}

@end

#pragma mark MLFeatureDescription

@interface MLFeatureDescription ()
- (instancetype)initWithName:(NSString *)name type:(MLFeatureType)type optional:(BOOL)optional;
@end

@implementation MLFeatureDescription

- (instancetype)initWithName:(NSString *)name type:(MLFeatureType)type optional:(BOOL)optional
{
	if ((self = [super init]) != nil) {
		_name = [name copy];
		_type = type;
		_optional = optional;
	}
	return self;
}

- (void)dealloc
{
	[_name release];
	[super dealloc];
}

- (BOOL)isAllowedValue:(MLFeatureValue *)value
{
	if (value.undefined)
		return _optional;
	return value.type == _type;
}

- (id)copyWithZone:(NSZone *)zone
{
	return [self retain];
}

@end

#pragma mark Feature providers

static MLFeatureValue *featureValueFromObject(id object)
{
	if ([object isKindOfClass:[MLFeatureValue class]])
		return object;
	if ([object isKindOfClass:[NSNumber class]]) {
		if (CFNumberIsFloatType((CFNumberRef) object))
			return [MLFeatureValue featureValueWithDouble:[object doubleValue]];
		return [MLFeatureValue featureValueWithInt64:[object longLongValue]];
	}
	if ([object isKindOfClass:[NSString class]])
		return [MLFeatureValue featureValueWithString:object];
	if ([object isKindOfClass:[MLMultiArray class]])
		return [MLFeatureValue featureValueWithMultiArray:object];
	// Other objects are kept as a value of invalid type.
	return [[[MLFeatureValue alloc] initWithType:MLFeatureTypeInvalid undefined:NO] autorelease];
}

@implementation MLDictionaryFeatureProvider

- (instancetype)initWithDictionary:(NSDictionary<NSString *, id> *)dictionary error:(NSError **)error
{
	NSMutableDictionary *converted = [NSMutableDictionary dictionaryWithCapacity:[dictionary count]];
	for (NSString *name in dictionary)
		converted[name] = featureValueFromObject(dictionary[name]);

	if ((self = [super init]) != nil)
		_dictionary = [converted copy];
	return self;
}

- (void)dealloc
{
	[_dictionary release];
	[super dealloc];
}

- (NSSet<NSString *> *)featureNames
{
	return [NSSet setWithArray:[_dictionary allKeys]];
}

- (MLFeatureValue *)featureValueForName:(NSString *)featureName
{
	return _dictionary[featureName];
}

- (MLFeatureValue *)objectForKeyedSubscript:(NSString *)featureName
{
	return _dictionary[featureName];
}

- (NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState *)state objects:(id *)buffer count:(NSUInteger)len
{
	return [_dictionary countByEnumeratingWithState:state objects:buffer count:len];
}

@end

@implementation MLArrayBatchProvider

- (instancetype)initWithFeatureProviderArray:(NSArray<id<MLFeatureProvider>> *)array
{
	if ((self = [super init]) != nil)
		_array = [array copy];
	return self;
}

- (instancetype)initWithDictionary:(NSDictionary<NSString *, NSArray *> *)dictionary error:(NSError **)error
{
	NSInteger count = -1;
	for (NSString *name in dictionary) {
		NSArray *values = dictionary[name];
		if (![values isKindOfClass:[NSArray class]] || (count >= 0 && (NSInteger) [values count] != count)) {
			failWithError(error, MLModelErrorGeneric, @"Every feature of a batch needs the same number of values");
			[self release];
			return nil;
		}
		count = (NSInteger) [values count];
	}

	NSMutableArray *providers = [NSMutableArray array];
	for (NSInteger i = 0; i < count; i++) {
		NSMutableDictionary *row = [NSMutableDictionary dictionary];
		for (NSString *name in dictionary)
			row[name] = dictionary[name][i];
		MLDictionaryFeatureProvider *provider = [[[MLDictionaryFeatureProvider alloc] initWithDictionary:row
		                                                                                          error:error] autorelease];
		if (provider == nil) {
			[self release];
			return nil;
		}
		[providers addObject:provider];
	}
	return [self initWithFeatureProviderArray:providers];
}

- (void)dealloc
{
	[_array release];
	[super dealloc];
}

- (NSInteger)count
{
	return (NSInteger) [_array count];
}

- (id<MLFeatureProvider>)featuresAtIndex:(NSInteger)index
{
	return _array[index];
}

@end

#pragma mark Models

@implementation MLModelConfiguration

- (instancetype)init
{
	if ((self = [super init]) != nil)
		_computeUnits = MLComputeUnitsAll;
	return self;
}

- (void)dealloc
{
	[_modelDisplayName release];
	[super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
	MLModelConfiguration *copy = [[[self class] allocWithZone:zone] init];
	copy->_modelDisplayName = [_modelDisplayName copy];
	copy->_computeUnits = _computeUnits;
	copy->_allowLowPrecisionAccumulationOnGPU = _allowLowPrecisionAccumulationOnGPU;
	return copy;
}

@end

@implementation MLPredictionOptions

- (instancetype)init
{
	if ((self = [super init]) != nil)
		_outputBackings = [@{} retain];
	return self;
}

- (void)setOutputBackings:(NSDictionary<NSString *, id> *)outputBackings
{
	if (_outputBackings != outputBackings) {
		[_outputBackings release];
		_outputBackings = [outputBackings copy];
	}
}

- (void)dealloc
{
	[_outputBackings release];
	[super dealloc];
}

@end

@implementation MLModelDescription

- (NSDictionary *)inputDescriptionsByName
{
	return @{};
}

- (NSDictionary *)outputDescriptionsByName
{
	return @{};
}

- (NSDictionary *)stateDescriptionsByName
{
	return @{};
}

- (NSDictionary *)trainingInputDescriptionsByName
{
	return @{};
}

- (NSDictionary *)metadata
{
	return @{};
}

- (NSString *)predictedFeatureName
{
	return nil;
}

- (NSString *)predictedProbabilitiesName
{
	return nil;
}

- (NSArray *)classLabels
{
	return nil;
}

- (BOOL)isUpdatable
{
	return NO;
}

@end

// Completion handlers run asynchronously, as they do on macOS.
static void completeLater(void (^handler)(id, NSError *))
{
	if (handler == nil)
		return;
	handler = [handler copy];
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		NSError *error = nil;
		failWithError(&error, MLModelErrorGeneric, kUnavailableDescription);
		handler(nil, error);
		[handler release];
	});
}

@implementation MLModel {
	MLModelDescription *_modelDescription;
	MLModelConfiguration *_configuration;
}

+ (instancetype)modelWithContentsOfURL:(NSURL *)url error:(NSError **)error
{
	failWithError(error, MLModelErrorGeneric, kUnavailableDescription);
	return nil;
}

+ (instancetype)modelWithContentsOfURL:(NSURL *)url configuration:(MLModelConfiguration *)configuration error:(NSError **)error
{
	failWithError(error, MLModelErrorGeneric, kUnavailableDescription);
	return nil;
}

+ (void)loadContentsOfURL:(NSURL *)url
            configuration:(MLModelConfiguration *)configuration
        completionHandler:(void (^)(MLModel *, NSError *))handler
{
	completeLater((void (^)(id, NSError *)) handler);
}

- (void)dealloc
{
	[_modelDescription release];
	[_configuration release];
	[super dealloc];
}

// No model can be loaded, so an instance has neither a description nor a
// configuration.
- (MLModelDescription *)modelDescription
{
	return _modelDescription;
}

- (MLModelConfiguration *)configuration
{
	return _configuration;
}

- (id<MLFeatureProvider>)predictionFromFeatures:(id<MLFeatureProvider>)input error:(NSError **)error
{
	failWithError(error, MLModelErrorGeneric, kUnavailableDescription);
	return nil;
}

- (id<MLFeatureProvider>)predictionFromFeatures:(id<MLFeatureProvider>)input
                                        options:(MLPredictionOptions *)options
                                          error:(NSError **)error
{
	failWithError(error, MLModelErrorGeneric, kUnavailableDescription);
	return nil;
}

- (void)predictionFromFeatures:(id<MLFeatureProvider>)input
             completionHandler:(void (^)(id<MLFeatureProvider>, NSError *))completionHandler
{
	completeLater((void (^)(id, NSError *)) completionHandler);
}

- (void)predictionFromFeatures:(id<MLFeatureProvider>)input
                       options:(MLPredictionOptions *)options
             completionHandler:(void (^)(id<MLFeatureProvider>, NSError *))completionHandler
{
	completeLater((void (^)(id, NSError *)) completionHandler);
}

- (id<MLBatchProvider>)predictionsFromBatch:(id<MLBatchProvider>)inputBatch error:(NSError **)error
{
	failWithError(error, MLModelErrorGeneric, kUnavailableDescription);
	return nil;
}

- (id<MLBatchProvider>)predictionsFromBatch:(id<MLBatchProvider>)inputBatch
                                    options:(MLPredictionOptions *)options
                                      error:(NSError **)error
{
	failWithError(error, MLModelErrorGeneric, kUnavailableDescription);
	return nil;
}

@end

#endif /* __LP64__ */
