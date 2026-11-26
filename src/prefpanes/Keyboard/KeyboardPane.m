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

// The Keyboard pane (com.apple.preference.keyboard), in the global preferences domain:
//  - KeyRepeat and InitialKeyRepeat, counted in 15 ms ticks: the interval between repeats and the delay
//    before the first one (the slider stops are the ones the system pane offers);
//  - com.apple.keyboard.fnState: the F1, F2, ... keys act as standard function keys;
//  - AppleKeyboardUIMode: 0 moves the focus between text boxes and lists only, 3 between all controls;
//  - NSAutomaticSpellingCorrectionEnabled, NSAutomaticCapitalizationEnabled,
//    NSAutomaticPeriodSubstitutionEnabled, NSAutomaticQuoteSubstitutionEnabled and
//    NSAutomaticDashSubstitutionEnabled: the text options that text views follow.
// The settings of Darling's own keyboard handling (which keys act as Command, ...) are in the Darling pane.

@interface KeyboardPreferencePane : PKPane
@end

@implementation KeyboardPreferencePane

- (NSArray<PKItem *> *)items
{
    NSMutableArray *items = [NSMutableArray array];
    [items addObject:[PKItem heading:@"Keyboard"]];
    [items addObject:[PKItem slider:@"KeyboardKeyRepeat" title:@"Key Repeat:" key:@"KeyRepeat"
                            values:@[@120, @90, @60, @30, @12, @6, @2] labels:@[@"Slow", @"Fast"] defaultValue:@6]];
    [items addObject:[PKItem slider:@"KeyboardInitialKeyRepeat" title:@"Delay Until Repeat:" key:@"InitialKeyRepeat"
                            values:@[@120, @94, @68, @35, @25, @15] labels:@[@"Long", @"Short"] defaultValue:@25]];
    [items addObject:[PKItem check:@"KeyboardFnState" title:@"Use F1, F2, etc. keys as standard function keys"
                               key:@"com.apple.keyboard.fnState" defaultValue:NO]];
    [items addObject:[PKItem popup:@"KeyboardFullAccess" title:@"Keyboard navigation:" key:@"AppleKeyboardUIMode"
                            values:@[@0, @3] titles:@[@"Move focus between text boxes and lists only", @"Move focus between all controls"]
                      defaultValue:@0]];

    [items addObject:[PKItem heading:@"Text"]];
    [items addObject:[PKItem check:@"KeyboardSpelling" title:@"Correct spelling automatically" key:@"NSAutomaticSpellingCorrectionEnabled" defaultValue:YES]];
    [items addObject:[PKItem check:@"KeyboardCapitalize" title:@"Capitalize words automatically" key:@"NSAutomaticCapitalizationEnabled" defaultValue:YES]];
    [items addObject:[PKItem check:@"KeyboardPeriod" title:@"Add period with double-space" key:@"NSAutomaticPeriodSubstitutionEnabled" defaultValue:YES]];
    [items addObject:[PKItem check:@"KeyboardQuotes" title:@"Use smart quotes" key:@"NSAutomaticQuoteSubstitutionEnabled" defaultValue:YES]];
    [items addObject:[PKItem check:@"KeyboardDashes" title:@"Use smart dashes" key:@"NSAutomaticDashSubstitutionEnabled" defaultValue:YES]];

    [items addObject:[PKItem heading:@"Darling"]];
    PKItem *link = [PKItem new];
    link.kind = PKKindView;
    NSView *holder = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 600, 26)];
    NSButton *button = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 330, 24)];
    [button setBezelStyle:NSBezelStyleRounded];
    [button setTitle:@"Open the keyboard settings of Darling…"];
    [button setToolTip:@"KeyboardDarlingLink"];
    [button setTarget:self];
    [button setAction:@selector(openDarlingKeyboardSettings:)];
    [holder addSubview:button];
    link.view = holder;
    [items addObject:link];
    return items;
}

- (void)openDarlingKeyboardSettings:(id)sender
{
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"x-apple.systempreferences:org.darlinghq.preference.darling?keyboard"]];
}

- (void)handleSystemPreferencesAnchor:(NSString *)anchor
{
}

@end
