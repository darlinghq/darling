/*
	AppSupport.private

	Apple's private framework that backs the common UI services: the
	address/city database, the radios preference pane, and a handful of
	CoreGraphics-adjacent helpers. Seven of the local apps link it, binding two
	classes and three functions.

	Every entry here is the neutral answer rather than a fabricated one. The two
	classes are objects with the accessors the callers use, returning nil, zero and
	NO; the logging function flushes to nothing, since there is no log store behind
	it; the two bitmap helpers return a failure, because inventing pixel data would
	be worse than reporting that the path could not be read.
*/

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>

/* ---- RadiosPreferences ------------------------------------------------------- */

@interface RadiosPreferences : NSObject
+ (id)sharedPreferences;
- (NSString*)radioStationNameForFrequency:(double)frequency;
- (BOOL)setRadioStationName:(NSString*)name forFrequency:(double)frequency;
- (NSArray*)knownStations;
@end

@implementation RadiosPreferences
+ (id)sharedPreferences
{
	static RadiosPreferences* instance;
	if (instance == nil)
	{
		instance = [[RadiosPreferences alloc] init];
	}
	return instance;
}

/* No station database is present, so there is no name for a frequency. Returning
   nil is what a caller already handles: it falls back to showing the frequency. */
- (NSString*)radioStationNameForFrequency:(double)frequency
{
	(void)frequency;
	return nil;
}

- (BOOL)setRadioStationName:(NSString*)name forFrequency:(double)frequency
{
	(void)name;
	(void)frequency;
	return NO;
}

- (NSArray*)knownStations
{
	return @[];
}
@end

/* ---- ALCityManager ----------------------------------------------------------- */

@interface ALCityManager : NSObject
+ (id)sharedManager;
- (NSString*)localizedCityNameForCoordinate:(double)latitude longitude:(double)longitude;
- (NSArray*)citiesNearCoordinate:(double)latitude longitude:(double)longitude withinMiles:(double)miles;
@end

@implementation ALCityManager
+ (id)sharedManager
{
	static ALCityManager* instance;
	if (instance == nil)
	{
		instance = [[ALCityManager alloc] init];
	}
	return instance;
}

/* No city database is present. An empty array rather than nil, so a caller that
   counts the results does not have to special-case a missing one. */
- (NSString*)localizedCityNameForCoordinate:(double)latitude longitude:(double)longitude
{
	(void)latitude;
	(void)longitude;
	return nil;
}

- (NSArray*)citiesNearCoordinate:(double)latitude longitude:(double)longitude withinMiles:(double)miles
{
	(void)latitude;
	(void)longitude;
	(void)miles;
	return @[];
}
@end

/* ---- C entry points ---------------------------------------------------------- */

void CPLoggingFlush(void)
{
	/* The log store is not present; flushing is a no-op rather than an error,
	   because every caller flushes unconditionally on a path that must not fail. */
}

CFArrayRef CPBitmapCreateImagesFromPath(CFStringRef path, CFDictionaryRef options)
{
	/* No bitmap reader behind this. NULL says the path could not be decoded, which
	   is a real answer; returning fabricated images would be worse. */
	(void)path;
	(void)options;
	return NULL;
}

Boolean CPBitmapWriteImagesToPath(CFArrayRef images, CFStringRef path, CFDictionaryRef options)
{
	(void)images;
	(void)path;
	(void)options;
	return false;
}
