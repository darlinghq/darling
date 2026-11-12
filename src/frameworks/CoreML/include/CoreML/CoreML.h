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


#ifndef _CoreML_H_
#define _CoreML_H_

#import <Foundation/Foundation.h>
#import <CoreVideo/CVPixelBuffer.h>

// Core ML for Darling: the model, configuration, feature and multi-array
// classes of the public API are present so that apps linking the framework
// load. There is no inference engine, so loading a model always fails with an
// MLModelErrorDomain error and predictions are never produced. Feature values,
// feature providers and multi-arrays are working value containers.
//
// Not declared: MLSequence, MLState, the constraint classes, MLParameterKey and
// the model update, compilation, compute device and model structure APIs.

#define ML_EXPORT extern __attribute__((visibility("default")))

NS_ASSUME_NONNULL_BEGIN

@class MLModelDescription;
@class MLFeatureDescription;
@class MLFeatureValue;
@class MLMultiArray;
@class MLPredictionOptions;
@class MLModelConfiguration;
@protocol MLFeatureProvider;
@protocol MLBatchProvider;

#pragma mark Errors

ML_EXPORT NSString *const MLModelErrorDomain;

typedef NS_ENUM(NSInteger, MLModelError) {
	MLModelErrorGeneric = 0,
	MLModelErrorFeatureType = 1,
	MLModelErrorIO = 3,
	MLModelErrorCustomLayer = 4,
	MLModelErrorCustomModel = 5,
	MLModelErrorUpdate = 6,
	MLModelErrorParameters = 7,
	MLModelErrorModelDecryptionKeyFetch = 8,
	MLModelErrorModelDecryption = 9,
	MLModelErrorModelCollection = 10,
	MLModelErrorPredictionCancelled = 11,
};

#pragma mark Metadata keys

typedef NSString *MLModelMetadataKey NS_STRING_ENUM;
ML_EXPORT MLModelMetadataKey const MLModelDescriptionKey;
ML_EXPORT MLModelMetadataKey const MLModelVersionStringKey;
ML_EXPORT MLModelMetadataKey const MLModelAuthorKey;
ML_EXPORT MLModelMetadataKey const MLModelLicenseKey;
ML_EXPORT MLModelMetadataKey const MLModelCreatorDefinedKey;

#pragma mark Features

typedef NS_ENUM(NSInteger, MLFeatureType) {
	MLFeatureTypeInvalid = 0,
	MLFeatureTypeInt64 = 1,
	MLFeatureTypeDouble = 2,
	MLFeatureTypeString = 3,
	MLFeatureTypeImage = 4,
	MLFeatureTypeMultiArray = 5,
	MLFeatureTypeDictionary = 6,
	MLFeatureTypeSequence = 7,
	MLFeatureTypeState = 8,
};

typedef NS_ENUM(NSInteger, MLMultiArrayDataType) {
	MLMultiArrayDataTypeDouble = 0x10000 | 64,
	MLMultiArrayDataTypeFloat64 = 0x10000 | 64,
	MLMultiArrayDataTypeFloat32 = 0x10000 | 32,
	MLMultiArrayDataTypeFloat16 = 0x10000 | 16,
	MLMultiArrayDataTypeFloat = 0x10000 | 32,
	MLMultiArrayDataTypeInt32 = 0x20000 | 32,
	MLMultiArrayDataTypeInt8 = 0x20000 | 8,
};

@interface MLMultiArray : NSObject
@property (readonly, nonatomic) void *dataPointer NS_RETURNS_INNER_POINTER;
@property (readonly, nonatomic) MLMultiArrayDataType dataType;
@property (readonly, nonatomic) NSArray<NSNumber *> *shape;
@property (readonly, nonatomic) NSArray<NSNumber *> *strides;
@property (readonly, nonatomic) NSInteger count;
@property (readonly, nullable, nonatomic) CVPixelBufferRef pixelBuffer;
- (nullable instancetype)initWithShape:(NSArray<NSNumber *> *)shape
                              dataType:(MLMultiArrayDataType)dataType
                                 error:(NSError **)error;
- (nullable instancetype)initWithDataPointer:(void *)dataPointer
                                       shape:(NSArray<NSNumber *> *)shape
                                    dataType:(MLMultiArrayDataType)dataType
                                     strides:(NSArray<NSNumber *> *)strides
                                 deallocator:(void (^_Nullable)(void *bytes))deallocator
                                       error:(NSError **)error;
- (void)getBytesWithHandler:(void (NS_NOESCAPE ^)(const void *bytes, NSInteger size))handler;
- (void)getMutableBytesWithHandler:(void (NS_NOESCAPE ^)(void *mutableBytes, NSInteger size, NSArray<NSNumber *> *strides))handler;
// The index is a position in row-major order over the shape; out of range
// indices raise NSInvalidArgumentException.
- (NSNumber *)objectAtIndexedSubscript:(NSInteger)idx;
- (NSNumber *)objectForKeyedSubscript:(NSArray<NSNumber *> *)key;
- (void)setObject:(NSNumber *)obj atIndexedSubscript:(NSInteger)idx;
- (void)setObject:(NSNumber *)obj forKeyedSubscript:(NSArray<NSNumber *> *)key;
@end

typedef NSString *MLFeatureValueImageOption NS_STRING_ENUM;
ML_EXPORT MLFeatureValueImageOption const MLFeatureValueImageOptionCropRect;
ML_EXPORT MLFeatureValueImageOption const MLFeatureValueImageOptionCropAndScale;

@interface MLFeatureValue : NSObject <NSCopying>
@property (readonly, nonatomic) MLFeatureType type;
@property (readonly, nonatomic, getter=isUndefined) BOOL undefined;
@property (readonly, nonatomic) int64_t int64Value;
@property (readonly, nonatomic) double doubleValue;
@property (readonly, nonatomic, copy) NSString *stringValue;
@property (readonly, nullable, nonatomic) MLMultiArray *multiArrayValue;
@property (readonly, nonatomic) NSDictionary<id, NSNumber *> *dictionaryValue;
@property (readonly, nullable, nonatomic) CVPixelBufferRef imageBufferValue;
+ (instancetype)featureValueWithInt64:(int64_t)value;
+ (instancetype)featureValueWithDouble:(double)value;
+ (instancetype)featureValueWithString:(NSString *)value;
+ (instancetype)featureValueWithMultiArray:(MLMultiArray *)value;
+ (instancetype)featureValueWithPixelBuffer:(CVPixelBufferRef)value;
+ (instancetype)undefinedFeatureValueWithType:(MLFeatureType)type;
+ (nullable instancetype)featureValueWithDictionary:(NSDictionary<id, NSNumber *> *)value error:(NSError **)error;
- (BOOL)isEqualToFeatureValue:(MLFeatureValue *)value;
@end

@interface MLFeatureDescription : NSObject <NSCopying>
@property (readonly, nonatomic, copy) NSString *name;
@property (readonly, nonatomic) MLFeatureType type;
@property (readonly, nonatomic, getter=isOptional) BOOL optional;
- (BOOL)isAllowedValue:(MLFeatureValue *)value;
@end

@protocol MLFeatureProvider
@property (readonly, nonatomic) NSSet<NSString *> *featureNames;
- (nullable MLFeatureValue *)featureValueForName:(NSString *)featureName;
@end

@interface MLDictionaryFeatureProvider : NSObject <MLFeatureProvider, NSFastEnumeration>
@property (readonly, nonatomic) NSDictionary<NSString *, MLFeatureValue *> *dictionary;
// MLFeatureValue, NSNumber, NSString and MLMultiArray values are converted;
// other objects are kept as values of type MLFeatureTypeInvalid.
- (nullable instancetype)initWithDictionary:(NSDictionary<NSString *, id> *)dictionary error:(NSError **)error;
- (nullable MLFeatureValue *)objectForKeyedSubscript:(NSString *)featureName;
@end

@protocol MLBatchProvider
@property (readonly, nonatomic) NSInteger count;
- (id<MLFeatureProvider>)featuresAtIndex:(NSInteger)index;
@end

@interface MLArrayBatchProvider : NSObject <MLBatchProvider>
@property (readonly, nonatomic) NSArray<id<MLFeatureProvider>> *array;
- (instancetype)initWithFeatureProviderArray:(NSArray<id<MLFeatureProvider>> *)array;
- (nullable instancetype)initWithDictionary:(NSDictionary<NSString *, NSArray *> *)dictionary error:(NSError **)error;
@end

#pragma mark Models

typedef NS_ENUM(NSInteger, MLComputeUnits) {
	MLComputeUnitsCPUOnly = 0,
	MLComputeUnitsCPUAndGPU = 1,
	MLComputeUnitsAll = 2,
	MLComputeUnitsCPUAndNeuralEngine = 3
};

@interface MLModelConfiguration : NSObject <NSCopying>
@property (readwrite, copy, nullable) NSString *modelDisplayName;
@property (readwrite) MLComputeUnits computeUnits;
@property (readwrite) BOOL allowLowPrecisionAccumulationOnGPU;
@end

@interface MLPredictionOptions : NSObject
@property (readwrite, nonatomic) BOOL usesCPUOnly;
@property (readwrite, copy, nonatomic) NSDictionary<NSString *, id> *outputBackings;
@end

@interface MLModelDescription : NSObject
@property (readonly, nonatomic) NSDictionary<NSString *, MLFeatureDescription *> *inputDescriptionsByName;
@property (readonly, nonatomic) NSDictionary<NSString *, MLFeatureDescription *> *outputDescriptionsByName;
@property (readonly, nonatomic) NSDictionary<NSString *, MLFeatureDescription *> *stateDescriptionsByName;
@property (readonly, nullable, nonatomic, copy) NSString *predictedFeatureName;
@property (readonly, nullable, nonatomic, copy) NSString *predictedProbabilitiesName;
@property (readonly, nonatomic) NSDictionary<MLModelMetadataKey, id> *metadata;
@property (readonly, nullable, nonatomic, copy) NSArray<id> *classLabels;
@property (readonly, nonatomic) BOOL isUpdatable;
@property (readonly, nonatomic) NSDictionary<NSString *, MLFeatureDescription *> *trainingInputDescriptionsByName;
@end

@interface MLModel : NSObject
@property (readonly, nonatomic) MLModelDescription *modelDescription;
@property (readonly, nonatomic) MLModelConfiguration *configuration;
// Loading always fails: the result is nil and the error is in MLModelErrorDomain
// with code MLModelErrorGeneric.
+ (nullable instancetype)modelWithContentsOfURL:(NSURL *)url error:(NSError **)error;
+ (nullable instancetype)modelWithContentsOfURL:(NSURL *)url
                                  configuration:(MLModelConfiguration *)configuration
                                          error:(NSError **)error;
+ (void)loadContentsOfURL:(NSURL *)url
            configuration:(MLModelConfiguration *)configuration
        completionHandler:(void (^)(MLModel *_Nullable model, NSError *_Nullable error))handler;
- (nullable id<MLFeatureProvider>)predictionFromFeatures:(id<MLFeatureProvider>)input error:(NSError **)error;
- (nullable id<MLFeatureProvider>)predictionFromFeatures:(id<MLFeatureProvider>)input
                                                 options:(MLPredictionOptions *)options
                                                   error:(NSError **)error;
- (void)predictionFromFeatures:(id<MLFeatureProvider>)input
             completionHandler:(void (^)(_Nullable id<MLFeatureProvider> output, NSError *_Nullable error))completionHandler;
- (void)predictionFromFeatures:(id<MLFeatureProvider>)input
                       options:(MLPredictionOptions *)options
             completionHandler:(void (^)(_Nullable id<MLFeatureProvider> output, NSError *_Nullable error))completionHandler;
- (nullable id<MLBatchProvider>)predictionsFromBatch:(id<MLBatchProvider>)inputBatch error:(NSError **)error;
- (nullable id<MLBatchProvider>)predictionsFromBatch:(id<MLBatchProvider>)inputBatch
                                             options:(MLPredictionOptions *)options
                                               error:(NSError **)error;
@end

NS_ASSUME_NONNULL_END

#endif
