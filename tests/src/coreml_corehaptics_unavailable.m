// CFLAGS: -framework Foundation -framework CoreML -framework CoreHaptics -framework CoreVideo -framework VideoToolbox
// Copyright (C) 2026 Lubos Dolezel
//
// An app that links CoreML and CoreHaptics must load and then get the
// documented "unavailable" results: no haptic hardware, no engine, no
// loadable model. Constants carry the string values of the public API. The
// test also references the CoreVideo and VideoToolbox constants and the Metal
// texture cache calls that were missing, and checks their values.
#import <Foundation/Foundation.h>
#import <CoreML/CoreML.h>
#import <CoreHaptics/CoreHaptics.h>
#import <CoreVideo/CoreVideo.h>
#import <VideoToolbox/VideoToolbox.h>
#include <dispatch/dispatch.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)
#define CHECK_STRING(sym, value) CHECK([(NSString *) (sym) isEqualToString:@value])

static void checkCoreHaptics(void)
{
	CHECK_STRING(CHHapticEventTypeHapticTransient, "HapticTransient");
	CHECK_STRING(CHHapticEventTypeHapticContinuous, "HapticContinuous");
	CHECK_STRING(CHHapticEventTypeAudioContinuous, "AudioContinuous");
	CHECK_STRING(CHHapticEventTypeAudioCustom, "AudioCustom");
	CHECK_STRING(CHHapticEventParameterIDHapticIntensity, "HapticIntensity");
	CHECK_STRING(CHHapticEventParameterIDHapticSharpness, "HapticSharpness");
	CHECK_STRING(CHHapticEventParameterIDAttackTime, "AttackTime");
	CHECK_STRING(CHHapticEventParameterIDDecayTime, "DecayTime");
	CHECK_STRING(CHHapticEventParameterIDReleaseTime, "ReleaseTime");
	CHECK_STRING(CHHapticEventParameterIDSustained, "Sustained");
	CHECK_STRING(CHHapticEventParameterIDAudioVolume, "AudioVolume");
	CHECK_STRING(CHHapticEventParameterIDAudioPitch, "AudioPitch");
	CHECK_STRING(CHHapticEventParameterIDAudioPan, "AudioPan");
	CHECK_STRING(CHHapticEventParameterIDAudioBrightness, "AudioBrightness");
	CHECK_STRING(CHHapticDynamicParameterIDHapticIntensityControl, "HapticIntensityControl");
	CHECK_STRING(CHHapticDynamicParameterIDHapticSharpnessControl, "HapticSharpnessControl");
	CHECK_STRING(CHHapticDynamicParameterIDHapticAttackTimeControl, "HapticAttackTimeControl");
	CHECK_STRING(CHHapticDynamicParameterIDHapticDecayTimeControl, "HapticDecayTimeControl");
	CHECK_STRING(CHHapticDynamicParameterIDHapticReleaseTimeControl, "HapticReleaseTimeControl");
	CHECK_STRING(CHHapticDynamicParameterIDAudioVolumeControl, "AudioVolumeControl");
	CHECK_STRING(CHHapticDynamicParameterIDAudioPanControl, "AudioPanControl");
	CHECK_STRING(CHHapticDynamicParameterIDAudioBrightnessControl, "AudioBrightnessControl");
	CHECK_STRING(CHHapticDynamicParameterIDAudioPitchControl, "AudioPitchControl");
	CHECK_STRING(CHHapticDynamicParameterIDAudioAttackTimeControl, "AudioAttackTimeControl");
	CHECK_STRING(CHHapticDynamicParameterIDAudioDecayTimeControl, "AudioDecayTimeControl");
	CHECK_STRING(CHHapticDynamicParameterIDAudioReleaseTimeControl, "AudioReleaseTimeControl");
	CHECK_STRING(CHHapticPatternKeyVersion, "Version");
	CHECK_STRING(CHHapticPatternKeyPattern, "Pattern");
	CHECK_STRING(CHHapticPatternKeyEvent, "Event");
	CHECK_STRING(CHHapticPatternKeyEventType, "EventType");
	CHECK_STRING(CHHapticPatternKeyTime, "Time");
	CHECK_STRING(CHHapticPatternKeyEventDuration, "EventDuration");
	CHECK_STRING(CHHapticPatternKeyEventWaveformPath, "EventWaveformPath");
	CHECK_STRING(CHHapticPatternKeyEventParameters, "EventParameters");
	CHECK_STRING(CHHapticPatternKeyEventWaveformUseVolumeEnvelope, "EventWaveformUseVolumeEnvelope");
	CHECK_STRING(CHHapticPatternKeyEventWaveformLoopEnabled, "EventWaveformLoopEnabled");
	CHECK_STRING(CHHapticPatternKeyParameter, "Parameter");
	CHECK_STRING(CHHapticPatternKeyParameterID, "ParameterID");
	CHECK_STRING(CHHapticPatternKeyParameterValue, "ParameterValue");
	CHECK_STRING(CHHapticPatternKeyParameterCurve, "ParameterCurve");
	CHECK_STRING(CHHapticPatternKeyParameterCurveControlPoints, "ParameterCurveControlPoints");
	CHECK_STRING(CHHapticAudioResourceKeyUseVolumeEnvelope, "UseVolumeEnvelope");
	CHECK_STRING(CHHapticAudioResourceKeyLoopEnabled, "LoopEnabled");
	CHECK_STRING(CoreHapticsErrorDomain, "com.apple.CoreHaptics");
	CHECK(CHHapticErrorCodeNotSupported == -4809);

	id<CHHapticDeviceCapability> capability = [CHHapticEngine capabilitiesForHardware];
	CHECK(capability != nil);
	CHECK(![capability supportsHaptics]);
	CHECK(![capability supportsAudio]);

	NSError *error = nil;
	CHHapticEngine *engine = [[CHHapticEngine alloc] initAndReturnError:&error];
	CHECK(engine == nil);
	CHECK([[error domain] isEqualToString:CoreHapticsErrorDomain]);
	CHECK([error code] == CHHapticErrorCodeNotSupported);

	// A NULL error pointer is allowed.
	CHECK([[CHHapticEngine alloc] initAndReturnError:NULL] == nil);

	// Patterns are value objects that can be built without an engine.
	CHHapticEventParameter *intensity = [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticIntensity
	                                                                                  value:0.5f];
	CHECK([intensity value] == 0.5f);
	CHHapticEvent *event = [[CHHapticEvent alloc] initWithEventType:CHHapticEventTypeHapticContinuous
	                                                     parameters:@[intensity]
	                                                   relativeTime:0.25
	                                                       duration:1.0];
	CHECK([[event type] isEqualToString:@"HapticContinuous"]);
	CHECK([[event eventParameters] count] == 1);
	error = nil;
	CHHapticPattern *pattern = [[CHHapticPattern alloc] initWithEvents:@[event] parameters:@[] error:&error];
	CHECK(pattern != nil);
	CHECK(error == nil);
	CHECK([pattern duration] == 1.25);
	[pattern release];
	[event release];
	[intensity release];
}

static void checkCoreML(void)
{
	CHECK_STRING(MLModelErrorDomain, "com.apple.CoreML");
	CHECK_STRING(MLModelDescriptionKey, "MLModelDescriptionKey");
	CHECK_STRING(MLModelVersionStringKey, "MLModelVersionStringKey");
	CHECK_STRING(MLModelAuthorKey, "MLModelAuthorKey");
	CHECK_STRING(MLModelLicenseKey, "MLModelLicenseKey");
	CHECK_STRING(MLModelCreatorDefinedKey, "MLModelCreatorDefinedKey");
	CHECK_STRING(MLFeatureValueImageOptionCropRect, "MLFeatureValueImageOptionCropRect");
	CHECK_STRING(MLFeatureValueImageOptionCropAndScale, "MLFeatureValueImageOptionCropAndScale");

	NSURL *url = [NSURL fileURLWithPath:@"/nonexistent.mlmodelc"];
	NSError *error = nil;
	MLModel *model = [MLModel modelWithContentsOfURL:url error:&error];
	CHECK(model == nil);
	CHECK([[error domain] isEqualToString:MLModelErrorDomain]);
	CHECK([error code] == MLModelErrorGeneric);

	error = nil;
	model = [MLModel modelWithContentsOfURL:url configuration:[[[MLModelConfiguration alloc] init] autorelease] error:&error];
	CHECK(model == nil);
	CHECK(error != nil);
	CHECK([MLModel modelWithContentsOfURL:url error:NULL] == nil);

	dispatch_semaphore_t sem = dispatch_semaphore_create(0);
	__block MLModel *asyncModel = (MLModel *) 1;
	__block NSError *asyncError = nil;
	[MLModel loadContentsOfURL:url
	             configuration:[[[MLModelConfiguration alloc] init] autorelease]
	         completionHandler:^(MLModel *m, NSError *e) {
		asyncModel = m;
		asyncError = [e retain];
		dispatch_semaphore_signal(sem);
	}];
	CHECK(dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC)) == 0);
	CHECK(asyncModel == nil);
	CHECK([[asyncError domain] isEqualToString:MLModelErrorDomain]);
	[asyncError release];

	// The configuration keeps what is set on it, and copies it.
	MLModelConfiguration *configuration = [[MLModelConfiguration alloc] init];
	CHECK([configuration computeUnits] == MLComputeUnitsAll);
	[configuration setComputeUnits:MLComputeUnitsCPUOnly];
	[configuration setModelDisplayName:@"name"];
	CHECK([configuration computeUnits] == MLComputeUnitsCPUOnly);
	MLModelConfiguration *copy = [configuration copy];
	CHECK([copy computeUnits] == MLComputeUnitsCPUOnly);
	CHECK([[copy modelDisplayName] isEqualToString:@"name"]);
	[copy release];
	[configuration release];

	// Multi-arrays store and convert their elements.
	error = nil;
	MLMultiArray *array = [[MLMultiArray alloc] initWithShape:@[@2, @3] dataType:MLMultiArrayDataTypeFloat32 error:&error];
	CHECK(array != nil);
	CHECK([array count] == 6);
	CHECK([[array strides] isEqualToArray:@[@3, @1]]);
	array[4] = @1.5;
	CHECK([array[4] floatValue] == 1.5f);
	CHECK([array[@[@1, @1]] floatValue] == 1.5f);
	CHECK([array[0] floatValue] == 0.0f);
	[array release];

	MLMultiArray *half = [[MLMultiArray alloc] initWithShape:@[@2] dataType:MLMultiArrayDataTypeFloat16 error:&error];
	half[0] = @0.5;
	half[1] = @(-2.0);
	CHECK([half[0] floatValue] == 0.5f);
	CHECK([half[1] floatValue] == -2.0f);
	[half release];

	error = nil;
	CHECK([[MLMultiArray alloc] initWithShape:@[@0] dataType:MLMultiArrayDataTypeDouble error:&error] == nil);
	CHECK(error != nil);

	// Feature values and providers.
	MLFeatureValue *fv = [MLFeatureValue featureValueWithInt64:7];
	CHECK([fv type] == MLFeatureTypeInt64);
	CHECK([fv int64Value] == 7);
	CHECK([[MLFeatureValue featureValueWithString:@"x"] type] == MLFeatureTypeString);
	CHECK([MLFeatureValue undefinedFeatureValueWithType:MLFeatureTypeDouble].undefined);

	error = nil;
	MLDictionaryFeatureProvider *provider = [[MLDictionaryFeatureProvider alloc] initWithDictionary:@{ @"a": @3, @"b": @"s", @"c": @2.5 }
	                                                                                           error:&error];
	CHECK(provider != nil);
	CHECK([[provider featureNames] isEqualToSet:[NSSet setWithObjects:@"a", @"b", @"c", nil]]);
	CHECK([[provider featureValueForName:@"a"] type] == MLFeatureTypeInt64);
	CHECK([[provider featureValueForName:@"c"] type] == MLFeatureTypeDouble);
	CHECK([[[provider featureValueForName:@"b"] stringValue] isEqualToString:@"s"]);
	[provider release];

	error = nil;
	CHECK([[MLDictionaryFeatureProvider alloc] initWithDictionary:@{ @"a": [NSDate date] } error:&error] == nil);
	CHECK([error code] == MLModelErrorFeatureType);

	error = nil;
	MLArrayBatchProvider *batch = [[MLArrayBatchProvider alloc] initWithDictionary:@{ @"a": @[@1, @2] } error:&error];
	CHECK([batch count] == 2);
	CHECK([[[batch featuresAtIndex:1] featureValueForName:@"a"] int64Value] == 2);
	[batch release];

	MLPredictionOptions *options = [[MLPredictionOptions alloc] init];
	CHECK([[options outputBackings] count] == 0);
	[options release];
}

static void checkCoreVideoAndVideoToolbox(void)
{
	CHECK_STRING(kCVPixelBufferCGImageCompatibilityKey, "CGImageCompatibility");
	CHECK_STRING(kCVPixelBufferCGBitmapContextCompatibilityKey, "CGBitmapContextCompatibility");
	CHECK_STRING(kCVMetalTextureCacheMaximumTextureAgeKey, "MaximumMetalTextureAge");
	CHECK_STRING(kCVMetalTextureUsage, "MetalTextureUsage");
	CHECK_STRING(kCVMetalTextureStorageMode, "MetalTextureStorageMode");
	CHECK_STRING(kVTDecompressionPropertyKey_RealTime, "RealTime");
	CHECK_STRING(kVTVideoEncoderList_CodecType, "CodecType");
	CHECK_STRING(kVTVideoEncoderList_EncoderName, "EncoderName");
	CHECK(kCVReturnUnsupported == -6663);

	CHECK(CVMetalTextureCacheGetTypeID() != CVMetalTextureGetTypeID());

	// No Metal interop: creation fails, validates its arguments first, and
	// leaves no object behind.
	NSObject *device = [[NSObject alloc] init];
	CVMetalTextureCacheRef cache = (CVMetalTextureCacheRef) 1;
	CHECK(CVMetalTextureCacheCreate(NULL, NULL, (id) device, NULL, &cache) == kCVReturnUnsupported);
	CHECK(cache == NULL);
	CHECK(CVMetalTextureCacheCreate(NULL, NULL, (id) device, NULL, NULL) == kCVReturnInvalidArgument);
	[device release];

	CVMetalTextureRef texture = (CVMetalTextureRef) 1;
	CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, (CVMetalTextureCacheRef) 1, (CVImageBufferRef) 1, NULL, 80, 16, 16, 0,
	                                                &texture) == kCVReturnUnsupported);
	CHECK(texture == NULL);
	CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, NULL, (CVImageBufferRef) 1, NULL, 80, 16, 16, 0, &texture)
	      == kCVReturnInvalidArgument);

	VTRegisterSupplementalVideoDecoderIfAvailable('avc1');
}

int main(void)
{
	@autoreleasepool {
		checkCoreHaptics();
		checkCoreML();
		checkCoreVideoAndVideoToolbox();
	}

	if (failures == 0)
		printf("PASS\n");
	return failures ? 1 : 0;
}
