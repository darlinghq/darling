/*
	SoftwareUpdate.private

	Apple's private framework behind the software-update controller. Boot Camp
	Assistant links it and binds one symbol, SUSoftwareUpdateController.

	Over NSObject: no updater exists here, so there is no more specific base worth
	naming. The class symbol is what the linker needs.
*/

#import <Foundation/Foundation.h>

@interface SUSoftwareUpdateController : NSObject
- (void)checkForUpdatesWithCompletion:(void (^)(id update))completion;
- (BOOL)applyUpdate:(id)update error:(NSError**)error;
@end
