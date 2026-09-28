/*
	GeoKit.private

	Apple's private framework behind the geocoding used by Photos and QuickTime
	Player to turn coordinates into a place name. Both link it, binding four
	symbols: two classes and two managed-object contexts.

	GEOCity and GeoKitPlace are the place objects; the contexts are the Core Data
	stack they are fetched from. There is no Core Data store and no geocoding
	service here, so the contexts are reported as absent, which is what a caller
	has to handle before it fetches anything, and the place objects carry no
	coordinates because there is nothing to place.

	GEOManagedObjectContext and GEODefaultManagedObjectContext are C globals
	holding a context. They are NULL, not fabricated empty contexts: a caller that
	sees NULL takes its no-location path, whereas a real empty context would look
	like "there are zero places here", which is a different and wrong answer.
*/

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>

/* No Core Data stack is present, so both contexts are absent. */
id GEOManagedObjectContext = NULL;
id GEODefaultManagedObjectContext = NULL;

@interface GeoKitPlace : NSObject
@property (readonly, copy) NSString* name;
@property (readonly) double latitude;
@property (readonly) double longitude;
@end

@implementation GeoKitPlace
{
	NSString* _name;
}
@synthesize name = _name;

/* No place was resolved, so there is no coordinate to report. Zero is the
   documented value for an unresolved location, and no caller treats it as a
   position. */
- (double)latitude
{
	return 0.0;
}

- (double)longitude
{
	return 0.0;
}
@end

@interface GEOCity : GeoKitPlace
@end

@implementation GEOCity
@end
