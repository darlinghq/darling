/*
	GeoKit.private

	Apple's private framework behind the geocoding Photos and QuickTime Player use
	to turn coordinates into a place name. Both link it, binding two classes and two
	managed-object contexts.

	The contexts are NULL rather than fabricated empty ones: a caller that sees NULL
	takes its no-location path, whereas a real empty context would read as "there
	are zero places here", which is a different and wrong answer.
*/

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>

extern id GEOManagedObjectContext;
extern id GEODefaultManagedObjectContext;

@interface GeoKitPlace : NSObject
@property (readonly, copy) NSString* name;
@property (readonly) double latitude;
@property (readonly) double longitude;
@end

@interface GEOCity : GeoKitPlace
@end
