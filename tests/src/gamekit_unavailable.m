// CFLAGS: -framework Foundation -framework AppKit -framework GameKit
// Copyright (C) 2026 Lubos Dolezel

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <GameKit/GameKit.h>
#include <stdio.h>

static int failures = 0;
#define check(cond) do { int ok_ = (cond) ? 1 : 0; printf("%s: %s\n", ok_ ? "PASS" : "FAIL", #cond); if (!ok_) failures++; } while (0)

int main(void)
{
	@autoreleasepool {
		// The error domain string and code are the ones macOS uses.
		check([GKErrorDomain isEqualToString:@"GKErrorDomain"]);
		check(GKErrorNotSupported == 16);

		// Game Center is unavailable: the local player is never authenticated.
		GKLocalPlayer *player = [GKLocalPlayer localPlayer];
		check(player != nil);
		check([GKLocalPlayer localPlayer] == player);
		check(![player isAuthenticated]);

		// Requests complete asynchronously with GKErrorNotSupported.
		__block NSError *loadError = nil;
		dispatch_semaphore_t sem = dispatch_semaphore_create(0);
		[GKAchievement loadAchievementsWithCompletionHandler:^(NSArray *achievements, NSError *error) {
			loadError = [error retain];
			dispatch_semaphore_signal(sem);
		}];
		check(dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC)) == 0);
		check([[loadError domain] isEqualToString:GKErrorDomain]);
		check([loadError code] == GKErrorNotSupported);

		__block NSError *reportError = nil;
		[GKAchievement reportAchievements:@[] withCompletionHandler:^(NSError *error) {
			reportError = [error retain];
			dispatch_semaphore_signal(sem);
		}];
		check(dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC)) == 0);
		check([reportError code] == GKErrorNotSupported);

		// A nil completion handler is accepted.
		[GKTurnBasedMatch loadMatchesWithCompletionHandler:nil];

		// The authenticate handler is called once with an error and no view controller.
		__block NSViewController *authController = (NSViewController *)1;
		__block NSError *authError = nil;
		player.authenticateHandler = ^(NSViewController *vc, NSError *error) {
			authController = vc;
			authError = [error retain];
			dispatch_semaphore_signal(sem);
		};
		// The handler is delivered on the main queue, so run the main loop.
		NSDate *deadline = [NSDate dateWithTimeIntervalSinceNow:5];
		while (authError == nil && [deadline timeIntervalSinceNow] > 0)
			[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
		check(authError != nil);
		check(authController == nil);
		check([authError code] == GKErrorNotSupported);

		// No panel can be presented.
		check(![[GKDialogController sharedDialogController] presentViewController:nil]);
	}
	return failures ? 1 : 0;
}
