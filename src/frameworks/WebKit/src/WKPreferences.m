/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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

#import <WebKit/WKPreferences.h>
#import "WKStubForwarding.h"

// The new implementation is 64-bit only, like WebKit2 on macOS; the 32-bit slice keeps the stub.
#if __LP64__

@implementation WKPreferences

@synthesize minimumFontSize = _minimumFontSize;
@synthesize javaScriptCanOpenWindowsAutomatically = _javaScriptCanOpenWindowsAutomatically;
@synthesize javaScriptEnabled = _javaScriptEnabled;
@synthesize tabFocusesLinks = _tabFocusesLinks;
@synthesize fraudulentWebsiteWarningEnabled = _fraudulentWebsiteWarningEnabled;
@synthesize siteSpecificQuirksModeEnabled = _siteSpecificQuirksModeEnabled;
@synthesize textInteractionEnabled = _textInteractionEnabled;
@synthesize elementFullscreenEnabled = _elementFullscreenEnabled;

- (instancetype)init {
    if ((self = [super init])) {
        _javaScriptEnabled = YES;
        _javaScriptCanOpenWindowsAutomatically = YES;
        _fraudulentWebsiteWarningEnabled = YES;
        _siteSpecificQuirksModeEnabled = YES;
        _textInteractionEnabled = YES;
    }
    return self;
}

WK_FORWARD_UNIMPLEMENTED_SELECTORS

@end

#else

// The properties and methods of the public headers have no implementation in the 32-bit stub.
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"

@implementation WKPreferences

WK_FORWARD_UNIMPLEMENTED_SELECTORS

@end

#endif
