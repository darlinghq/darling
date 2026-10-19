/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

/*
 * The smallest iOS-shaped application that exercises the UIKit surface: an
 * application delegate, a hosting window, a root view controller and one label.
 * No storyboard, no scene manifest, no Swift, no Auto Layout.
 *
 * Provenance: Apple's public UIApplicationDelegate/UIWindow/UIViewController/
 * UILabel documentation (clean-room rung 3).  The shape is hand-written
 * fixture code, not a template or a copied application bundle.
 *
 * AppKit appears only in the fixture's exit path: the timer that bounds the run
 * has to stop the AppKit run loop, and UIKit has no public "quit" on iOS.
 */

#import <UIKit/UIKit.h>
#import <AppKit/NSApplication.h>

#include <stdio.h>

@interface HelloAppDelegate : UIResponder <UIApplicationDelegate>
@property (nonatomic, strong) UIWindow *window;
- (void)finishFixture:(NSTimer *)timer;
@end

@implementation HelloAppDelegate

- (BOOL)application:(UIApplication *)application
        didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	UIWindow *window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	window.backgroundColor = [UIColor whiteColor];
	UIViewController *controller = [[UIViewController alloc] init];

	UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(40, 40, 320, 48)];
	label.text = @"Hello from UIKit";
	label.font = [UIFont boldSystemFontOfSize:28];
	label.textColor = [UIColor redColor];
	[controller.view addSubview:label];

	window.rootViewController = controller;
	self.window = window;
	[window makeKeyAndVisible];

	printf("OK: didFinishLaunching, window ordered, label frame %.0fx%.0f at %.0f,%.0f\n",
	       label.frame.size.width, label.frame.size.height,
	       label.frame.origin.x, label.frame.origin.y);

	/* Bounded run: proves the run loop delivered the callback, then exits. */
	[NSTimer scheduledTimerWithTimeInterval:20.0
	                                 target:self
	                               selector:@selector(finishFixture:)
	                               userInfo:nil
	                                repeats:NO];
	return YES;
}

- (void)finishFixture:(NSTimer *)timer
{
	puts("OK: fixture timer fired; requesting termination");
	[NSApp terminate:nil];
}

@end

int main(int argc, char *argv[])
{
	setbuf(stdout, NULL);
	int status = UIApplicationMain(argc, argv, nil, @"HelloAppDelegate");
	printf("PASS: UIApplicationMain returned %d\n", status);
	return status;
}
