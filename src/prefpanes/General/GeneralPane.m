/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#import "PKPane.h"
#import <CoreServices/CoreServices.h>

// The General pane (com.apple.preference.general): the look of the interface and a few behaviours of
// applications, kept in the global preferences domain that applications read:
//  - AppleInterfaceStyle ("Dark", absent for the light appearance), announced with the distributed
//    notification AppleInterfaceThemeChangedNotification;
//  - AppleAccentColor (-1 graphite, 0 red, 1 orange, 2 yellow, 3 green, absent blue, 5 purple, 6 pink),
//    AppleAquaColorVariant (6 for graphite) and AppleHighlightColor ("r g b Name"), announced with
//    AppleColorPreferencesChangedNotification;
//  - AppleShowScrollBars (Automatic, WhenScrolling, Always), announced with AppleShowScrollBarsSettingChanged,
//    and AppleScrollerPagingBehavior (0 pages, 1 jumps to the clicked spot);
//  - NSTableViewDefaultSizeMode (1, 2, 3: the size of sidebar icons and text);
//  - NSQuitAlwaysKeepsWindows (the "Close windows when quitting" box is its inverse) and
//    NSCloseAlwaysConfirmsChanges;
//  - the number of recent items, com.apple.recentitems;
//  - the default web browser, kept by Launch Services for the http and https schemes;
//  - AppleFontSmoothing (0 off, 1 light, 2 medium, 3 strong).
// The keys and values are the ones reported in public write-ups of `defaults` for macOS; AppKit reacts to
// the first group of notifications (see NSApplication).

@interface GeneralPreferencePane : PKPane
@end

static NSColor *RGB(double r, double g, double b)
{
    return [NSColor colorWithCalibratedRed:r green:g blue:b alpha:1];
}

static NSString *ApplicationName(NSString *bundleIdentifier)
{
    CFArrayRef urls = LSCopyApplicationURLsForBundleIdentifier((__bridge CFStringRef)bundleIdentifier, NULL);
    NSURL *url = [(__bridge_transfer NSArray *)urls firstObject];
    if (url == nil)
        return bundleIdentifier;
    return [[NSFileManager defaultManager] displayNameAtPath:[url path]];
}

static NSArray<NSString *> *WebBrowsers(void)
{
    NSMutableArray *browsers = [NSMutableArray array];
    for (NSString *scheme in @[@"http", @"https"]) {
        for (NSString *identifier in (__bridge_transfer NSArray *)LSCopyAllHandlersForURLScheme((__bridge CFStringRef)scheme)) {
            if (![browsers containsObject:identifier])
                [browsers addObject:identifier];
        }
    }
    return browsers;
}

// The number of recent items is the MaxAmount of the Documents, Applications and Hosts dictionaries of
// com.apple.recentitems: how many of each kind the Apple menu lists.
static PKItem *RecentItem(void)
{
    NSArray *kinds = @[@"Documents", @"Applications", @"Hosts"];
    PKItem *item = [PKItem popup:@"GeneralRecentItems" title:@"Recent items:" key:nil
                          values:@[@0, @5, @10, @15, @20, @30, @50]
                          titles:@[@"None", @"5", @"10", @"15", @"20", @"30", @"50"] defaultValue:@10];
    item.help = @"The number of documents, applications and servers in the Recent Items menu.";
    item.readBlock = ^id{
        NSDictionary *dictionary = [PKPane preferenceForKey:@"Documents" domain:@"com.apple.recentitems" currentHost:NO];
        return [dictionary isKindOfClass:[NSDictionary class]] ? dictionary[@"MaxAmount"] : nil;
    };
    item.writeBlock = ^(id value) {
        for (NSString *kind in kinds) {
            NSMutableDictionary *dictionary = [NSMutableDictionary dictionary];
            id old = [PKPane preferenceForKey:kind domain:@"com.apple.recentitems" currentHost:NO];
            if ([old isKindOfClass:[NSDictionary class]])
                [dictionary addEntriesFromDictionary:old];
            if (value != nil)
                dictionary[@"MaxAmount"] = value;
            else
                [dictionary removeObjectForKey:@"MaxAmount"];
            [PKPane setPreference:([dictionary count] != 0 ? dictionary : nil) forKey:kind domain:@"com.apple.recentitems" currentHost:NO];
        }
    };
    return item;
}

@implementation GeneralPreferencePane

- (NSArray<PKItem *> *)items
{
    NSMutableArray *items = [NSMutableArray array];

    PKItem *appearance = [PKItem popup:@"GeneralAppearance" title:@"Appearance:" key:@"AppleInterfaceStyle"
                                values:@[[NSNull null], @"Dark"] titles:@[@"Light", @"Dark"] defaultValue:nil];
    appearance.notification = @"AppleInterfaceThemeChangedNotification";
    [items addObject:appearance];

    PKItem *accent = [PKItem new];
    accent.kind = PKKindSwatches;
    accent.identifier = @"GeneralAccent";
    accent.title = @"Accent color:";
    accent.key = @"AppleAccentColor";
    accent.values = @[[NSNull null], @5, @6, @0, @1, @2, @3, @(-1)];
    accent.colors = @[RGB(0.0, 0.48, 1.0), RGB(0.69, 0.32, 0.87), RGB(1.0, 0.18, 0.45), RGB(1.0, 0.23, 0.19),
                      RGB(1.0, 0.58, 0.0), RGB(1.0, 0.80, 0.0), RGB(0.20, 0.78, 0.35), RGB(0.60, 0.60, 0.62)];
    accent.notification = @"AppleColorPreferencesChangedNotification";
    [items addObject:accent];

    PKItem *highlight = [PKItem popup:@"GeneralHighlight" title:@"Highlight color:" key:@"AppleHighlightColor"
                               values:@[[NSNull null], @"0.847059 0.847059 0.862745 Graphite", @"1.000000 0.733333 0.721569 Red",
                                        @"1.000000 0.874510 0.701961 Orange", @"1.000000 0.937255 0.690196 Yellow",
                                        @"0.752941 0.964706 0.678431 Green", @"0.968627 0.831373 1.000000 Purple",
                                        @"1.000000 0.749020 0.874510 Pink"]
                               titles:@[@"Blue", @"Graphite", @"Red", @"Orange", @"Yellow", @"Green", @"Purple", @"Pink"] defaultValue:nil];
    highlight.notification = @"AppleColorPreferencesChangedNotification";
    [items addObject:highlight];

    [items addObject:[PKItem popup:@"GeneralSidebarIconSize" title:@"Sidebar icon size:" key:@"NSTableViewDefaultSizeMode"
                            values:@[@1, @2, @3] titles:@[@"Small", @"Medium", @"Large"] defaultValue:@2]];

    PKItem *scrollBars = [PKItem popup:@"GeneralScrollBars" title:@"Show scroll bars:" key:@"AppleShowScrollBars"
                                values:@[@"Automatic", @"WhenScrolling", @"Always"]
                                titles:@[@"Automatically based on mouse or trackpad", @"When scrolling", @"Always"] defaultValue:@"Automatic"];
    scrollBars.notification = @"AppleShowScrollBarsSettingChanged";
    [items addObject:scrollBars];

    [items addObject:[PKItem popup:@"GeneralScrollPaging" title:@"Click in the scroll bar to:" key:@"AppleScrollerPagingBehavior"
                            values:@[@0, @1] titles:@[@"Jump to the next page", @"Jump to the spot that's clicked"] defaultValue:@0]];

    NSArray<NSString *> *browsers = WebBrowsers();
    PKItem *browser = [PKItem popup:@"GeneralBrowser" title:@"Default web browser:" key:nil
                             values:([browsers count] != 0 ? browsers : @[[NSNull null]])
                             titles:nil defaultValue:nil];
    NSMutableArray *browserTitles = [NSMutableArray array];
    for (NSString *identifier in browsers)
        [browserTitles addObject:ApplicationName(identifier)];
    if ([browserTitles count] == 0)
        [browserTitles addObject:@"No web browser installed"];
    browser.titles = browserTitles;
    browser.readBlock = ^id{
        return CFBridgingRelease(LSCopyDefaultHandlerForURLScheme(CFSTR("http")));
    };
    browser.writeBlock = ^(id value) {
        if (value != nil) {
            LSSetDefaultHandlerForURLScheme(CFSTR("http"), (__bridge CFStringRef)value);
            LSSetDefaultHandlerForURLScheme(CFSTR("https"), (__bridge CFStringRef)value);
        }
    };
    [items addObject:browser];

    [items addObject:RecentItem()];

    PKItem *close = [PKItem check:@"GeneralCloseWindowsOnQuit" title:@"Close windows when quitting an application"
                              key:@"NSQuitAlwaysKeepsWindows" defaultValue:NO];
    close.inverted = YES;
    close.help = @"When selected, open documents and windows will not be restored when you re-open an application.";
    [items addObject:close];

    [items addObject:[PKItem check:@"GeneralAskToKeepChanges" title:@"Ask to keep changes when closing documents"
                               key:@"NSCloseAlwaysConfirmsChanges" defaultValue:NO]];

    PKItem *smoothing = [PKItem popup:@"GeneralFontSmoothing" title:@"Font smoothing:" key:@"AppleFontSmoothing"
                               values:@[[NSNull null], @0, @1, @2, @3]
                               titles:@[@"Default", @"Off", @"Light", @"Medium", @"Strong"] defaultValue:nil];
    smoothing.help = @"Applies to applications opened after the change.";
    [items addObject:smoothing];
    return items;
}

- (void)itemDidChange:(PKItem *)item toValue:(id)value
{
    // the graphite appearance of the controls is a colour variant that goes with the graphite accent
    if ([item.identifier isEqualToString:@"GeneralAccent"])
        [PKPane setPreference:([value isEqual:@(-1)] ? @6 : nil) forKey:@"AppleAquaColorVariant" domain:nil currentHost:NO];
}

- (void)handleSystemPreferencesAnchor:(NSString *)anchor
{
}

@end
