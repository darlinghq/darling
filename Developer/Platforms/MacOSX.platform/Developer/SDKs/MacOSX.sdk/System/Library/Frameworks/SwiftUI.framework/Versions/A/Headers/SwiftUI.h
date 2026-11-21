// Copyright © 2015 Apple Inc. All rights reserved.

#import <TargetConditionals.h>

#if !__METAL_VERSION__

#if TARGET_OS_OSX && __has_include(<AppKit/AppKit.h>)
#import <AppKit/AppKit.h>
#endif

#if !TARGET_OS_OSX && __has_include(<UIKit/UIKit.h>)
#import <UIKit/UIKit.h>
#endif

#if TARGET_OS_WATCH && __has_include(<WatchKit/WatchKit.h>)
#import <WatchKit/WatchKit.h>
#endif

#else
#include <SwiftUI/SwiftUI_Metal.h>
#endif
