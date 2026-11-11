/*
	SoftwareUpdate.private

	Apple's private framework behind the software-update controller. Boot Camp
	Assistant links it, and binds one symbol: SUSoftwareUpdateController.

	The controller is what drives "check for updates" in a privileged tool: it
	checks, tells the caller what it found, and applies an update the caller hands
	it. Nothing here is updatable and no update catalogue is reachable, so checking
	completes immediately reporting no update available, which is the answer a
	build with nothing to update gives.
*/

#import <Foundation/Foundation.h>

@interface SUSoftwareUpdateController : NSObject

/* Whether an update is available. NO, and completion is called with nil. */
- (void)checkForUpdatesWithCompletion:(void (^)(id update))completion;

/* Apply an update the caller found. Reports NO: there is no updater here to hand
   it to, which is a real answer rather than a silent success. */
- (BOOL)applyUpdate:(id)update error:(NSError**)error;

@end

@implementation SUSoftwareUpdateController

- (void)checkForUpdatesWithCompletion:(void (^)(id update))completion
{
	if (completion)
	{
		completion(nil);
	}
}

- (BOOL)applyUpdate:(id)update error:(NSError**)error
{
	(void)update;
	if (error)
	{
		*error = nil;
	}
	return NO;
}

@end
