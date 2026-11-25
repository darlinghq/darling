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

#import <Cocoa/Cocoa.h>
#import <PreferencePanes/PreferencePanes.h>
#import "DPSection.h"

// The Darling pane of System Preferences: the settings of Darling that are otherwise reached
// with `defaults write`, grouped in tabs. A tab's controls read the settings through
// CFPreferences and show the documented default while a key is not set; choosing "Default"
// (or clearing a field) removes the key.
//
// User settings are written when the control changes. System-wide settings (the platform
// identity, the security configuration and what Metal reports) are edited first and written
// by Apply.

static const CGFloat kPaneWidth = 640;
static const CGFloat kMargin = 16;
static const CGFloat kLabelWidth = 220;
static const CGFloat kControlX = 244;
static const CGFloat kControlWidth = 250;
static const CGFloat kHelpWidth = 360;
static const CGFloat kStatusHeight = 30;

@interface DPFlippedView : NSView
@end
@implementation DPFlippedView
- (BOOL)isFlipped { return YES; }
@end

// A control of a setting
@interface DPRow : NSObject
@property (strong) DPSetting *setting;
@property (strong) NSControl *control;
@property (strong) DPSection *section;
@property BOOL perApplication;     // the setting of the application chosen in the Applications tab
@end
@implementation DPRow
@end

@interface DarlingPreferencePane : NSPreferencePane <NSTabViewDelegate, NSTextFieldDelegate> {
    NSTabView *_tabs;
    NSTextField *_status;
    NSMutableArray<DPRow *> *_rows;
    NSMutableArray<NSNumber *> *_contentHeights;       // by tab
    NSMutableDictionary<NSString *, NSMutableDictionary *> *_pending;   // section identifier -> key -> @[setting, value or NSNull]
    NSMutableDictionary<NSString *, NSButton *> *_applyButtons, *_revertButtons;

    NSPopUpButton *_applicationPopup;
    NSTextField *_applicationField;
    NSArray<NSDictionary *> *_installedApplications;
    NSString *_applicationDomain;
}
- (void)handleSystemPreferencesAnchor:(NSString *)anchor;
@end

@implementation DarlingPreferencePane

#pragma mark Building the view

static NSTextField *MakeLabel(NSString *text, NSRect frame, NSFont *font, NSColor *color, NSTextAlignment alignment)
{
    NSTextField *label = [[NSTextField alloc] initWithFrame:frame];
    [label setStringValue:text ?: @""];
    [label setBezeled:NO];
    [label setBordered:NO];
    [label setDrawsBackground:NO];
    [label setEditable:NO];
    [label setSelectable:NO];
    [label setFont:font];
    [label setTextColor:color];
    [label setAlignment:alignment];
    return label;
}

static CGFloat HeightOfText(NSString *text, NSFont *font, CGFloat width)
{
    NSRect box = [text boundingRectWithSize:NSMakeSize(width, 10000)
                                    options:NSStringDrawingUsesLineFragmentOrigin
                                 attributes:@{ NSFontAttributeName: font }];
    return ceil(box.size.height) + 2;
}

// Adds wrapped small text to the view at y, returns the y below it.
- (CGFloat)addCaption:(NSString *)text toView:(NSView *)view x:(CGFloat)x y:(CGFloat)y width:(CGFloat)width
{
    NSFont *font = [NSFont systemFontOfSize:[NSFont smallSystemFontSize]];
    // the text cell wraps a few points inside its frame
    CGFloat height = HeightOfText(text, font, width - 8) + 4;
    NSTextField *label = MakeLabel(text, NSMakeRect(x, y, width, height), font, [NSColor grayColor], NSTextAlignmentLeft);
    [[label cell] setWraps:YES];
    [view addSubview:label];
    return y + height;
}

- (DPRow *)addRowForSetting:(DPSetting *)setting section:(DPSection *)section perApplication:(BOOL)perApplication
                     toView:(NSView *)view y:(CGFloat *)yPointer
{
    CGFloat y = *yPointer;
    DPRow *row = [[DPRow alloc] init];
    row.setting = setting;
    row.section = section;
    row.perApplication = perApplication;

    NSFont *font = [NSFont systemFontOfSize:[NSFont systemFontSize]];
    [view addSubview:MakeLabel([setting.title stringByAppendingString:@":"], NSMakeRect(kMargin, y + 3, kLabelWidth, 17),
                               font, [NSColor blackColor], NSTextAlignmentRight)];

    CGFloat controlHeight = 22;
    if (setting.kind == DPKindBool || setting.kind == DPKindChoice) {
        NSPopUpButton *popup = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(kControlX, y, kControlWidth, controlHeight) pullsDown:NO];
        NSMutableArray *titles = [[setting popupTitles] mutableCopy];
        if (perApplication)
            titles[0] = NSLocalizedString(@"Same as for all applications", nil);
        [popup addItemsWithTitles:titles];
        [popup setTarget:self];
        [popup setAction:@selector(popupChanged:)];
        row.control = popup;
    } else {
        NSTextField *field = [[NSTextField alloc] initWithFrame:NSMakeRect(kControlX, y, setting.unit != nil ? 170 : kControlWidth, controlHeight)];
        [field setEditable:YES];
        [field setBezeled:YES];
        [field setDelegate:self];
        [[field cell] setPlaceholderString:setting.placeholder ?: setting.defaultText];
        row.control = field;
        if (setting.unit != nil)
            [view addSubview:MakeLabel(setting.unit, NSMakeRect(kControlX + 176, y + 3, 60, 17), font, [NSColor grayColor], NSTextAlignmentLeft)];
    }
    [row.control setTag:(NSInteger)[_rows count]];
    [row.control setToolTip:setting.key];
    [view addSubview:row.control];
    [_rows addObject:row];

    y += controlHeight + 3;
    if ([setting.help length] != 0 && !perApplication)
        y = [self addCaption:setting.help toView:view x:kControlX y:y width:kHelpWidth];
    *yPointer = y + 12;
    return row;
}

- (NSView *)viewForSection:(DPSection *)section
{
    NSView *view = [[DPFlippedView alloc] initWithFrame:NSMakeRect(0, 0, kPaneWidth - 20, 100)];
    CGFloat y = 12;
    CGFloat fullWidth = kPaneWidth - 20 - 2 * kMargin;
    y = [self addCaption:section.note toView:view x:kMargin y:y width:fullWidth] + 14;

    for (DPSetting *setting in section.userSettings)
        [self addRowForSetting:setting section:section perApplication:NO toView:view y:&y];

    if ([section.identifier isEqualToString:@"applications"])
        [self addApplicationControlsToView:view section:section y:&y];

    if ([section.systemSettings count] != 0) {
        if ([section.userSettings count] != 0) {
            NSBox *line = [[NSBox alloc] initWithFrame:NSMakeRect(kMargin, y, fullWidth, 1)];
            [line setBoxType:NSBoxSeparator];
            [view addSubview:line];
            y += 12;
        }
        if ([section.systemNote length] != 0)
            y = [self addCaption:section.systemNote toView:view x:kMargin y:y width:fullWidth] + 14;
        for (DPSetting *setting in section.systemSettings)
            [self addRowForSetting:setting section:section perApplication:NO toView:view y:&y];

        NSButton *apply = [[NSButton alloc] initWithFrame:NSMakeRect(kPaneWidth - 20 - kMargin - 90, y, 90, 26)];
        [apply setTitle:NSLocalizedString(@"Apply", nil)];
        [apply setBezelStyle:NSBezelStyleRounded];
        [apply setTarget:self];
        [apply setAction:@selector(apply:)];
        [apply setEnabled:NO];
        [apply setToolTip:section.identifier];
        [view addSubview:apply];
        _applyButtons[section.identifier] = apply;

        NSButton *revert = [[NSButton alloc] initWithFrame:NSMakeRect(kPaneWidth - 20 - kMargin - 90 - 100, y, 90, 26)];
        [revert setTitle:NSLocalizedString(@"Revert", nil)];
        [revert setBezelStyle:NSBezelStyleRounded];
        [revert setTarget:self];
        [revert setAction:@selector(revert:)];
        [revert setEnabled:NO];
        [revert setToolTip:section.identifier];
        [view addSubview:revert];
        _revertButtons[section.identifier] = revert;
        y += 26 + 12;
    }

    [view setFrameSize:NSMakeSize(kPaneWidth - 20, y + 4)];
    return view;
}

#pragma mark Applications tab

static NSString *DisplayNameOfBundle(NSBundle *bundle, NSString *path)
{
    for (NSString *key in @[ @"CFBundleDisplayName", @"CFBundleName" ]) {
        id name = [bundle objectForInfoDictionaryKey:key];
        if ([name isKindOfClass:[NSString class]] && [name length] != 0)
            return name;
    }
    return [[path lastPathComponent] stringByDeletingPathExtension];
}

// The applications of the usual folders, by name. (The user can type a bundle identifier for
// anything else.)
- (NSArray<NSDictionary *> *)scanInstalledApplications
{
    NSMutableArray *found = [NSMutableArray array];
    NSMutableSet *seen = [NSMutableSet set];
    NSFileManager *fm = [NSFileManager defaultManager];
    NSArray *folders = @[ @"/Applications", @"/Applications/Utilities", @"/System/Applications",
                          [NSHomeDirectory() stringByAppendingPathComponent:@"Applications"] ];
    for (NSString *folder in folders) {
        for (NSString *name in [fm contentsOfDirectoryAtPath:folder error:nil]) {
            if (![[name pathExtension] isEqualToString:@"app"])
                continue;
            NSString *path = [folder stringByAppendingPathComponent:name];
            NSBundle *bundle = [NSBundle bundleWithPath:path];
            NSString *identifier = [bundle bundleIdentifier];
            if ([identifier length] == 0 || [seen containsObject:identifier])
                continue;
            [seen addObject:identifier];
            [found addObject:@{ @"name": DisplayNameOfBundle(bundle, path), @"identifier": identifier }];
        }
    }
    [found sortUsingComparator:^NSComparisonResult(NSDictionary *a, NSDictionary *b) {
        return [a[@"name"] caseInsensitiveCompare:b[@"name"]];
    }];
    return found;
}

- (void)addApplicationControlsToView:(NSView *)view section:(DPSection *)section y:(CGFloat *)yPointer
{
    CGFloat y = *yPointer;
    CGFloat fullWidth = kPaneWidth - 20 - 2 * kMargin;
    NSBox *line = [[NSBox alloc] initWithFrame:NSMakeRect(kMargin, y, fullWidth, 1)];
    [line setBoxType:NSBoxSeparator];
    [view addSubview:line];
    y += 12;
    y = [self addCaption:NSLocalizedString(@"An application's own value wins over the settings for all applications. Choose an application, or type the bundle identifier of any application, and give it values of its own; \"Same as for all applications\" removes them again.", nil)
                  toView:view x:kMargin y:y width:fullWidth] + 12;

    NSFont *font = [NSFont systemFontOfSize:[NSFont systemFontSize]];
    [view addSubview:MakeLabel(NSLocalizedString(@"Application:", nil), NSMakeRect(kMargin, y + 3, kLabelWidth, 17), font, [NSColor blackColor], NSTextAlignmentRight)];
    _applicationPopup = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(kControlX, y, kControlWidth + 60, 22) pullsDown:NO];
    _installedApplications = [self scanInstalledApplications];
    [_applicationPopup addItemWithTitle:NSLocalizedString(@"Choose an application…", nil)];
    for (NSDictionary *app in _installedApplications)
        [_applicationPopup addItemWithTitle:[NSString stringWithFormat:@"%@ (%@)", app[@"name"], app[@"identifier"]]];
    [_applicationPopup setTarget:self];
    [_applicationPopup setAction:@selector(applicationChosen:)];
    [view addSubview:_applicationPopup];
    y += 22 + 8;

    [view addSubview:MakeLabel(NSLocalizedString(@"Bundle identifier:", nil), NSMakeRect(kMargin, y + 3, kLabelWidth, 17), font, [NSColor blackColor], NSTextAlignmentRight)];
    _applicationField = [[NSTextField alloc] initWithFrame:NSMakeRect(kControlX, y, kControlWidth + 60, 22)];
    [[_applicationField cell] setPlaceholderString:@"com.example.Application"];
    [_applicationField setDelegate:self];
    [_applicationField setToolTip:@"ApplicationIdentifier"];
    [view addSubview:_applicationField];
    y += 22 + 14;

    for (DPSetting *setting in [DPSection applicationSettings]) {
        DPRow *row = [self addRowForSetting:setting section:section perApplication:YES toView:view y:&y];
        [row.control setEnabled:NO];
    }
    *yPointer = y;
}

- (NSString *)validBundleIdentifier:(NSString *)text
{
    NSString *trimmed = [text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    NSMutableCharacterSet *allowed = [NSMutableCharacterSet alphanumericCharacterSet];
    [allowed addCharactersInString:@".-_"];
    if ([trimmed length] == 0 || [trimmed rangeOfCharacterFromSet:[allowed invertedSet]].location != NSNotFound)
        return nil;
    return trimmed;
}

- (void)selectApplicationDomain:(NSString *)domain
{
    _applicationDomain = [domain copy];
    for (DPRow *row in _rows) {
        if (!row.perApplication)
            continue;
        [row.control setEnabled:domain != nil];
        [self refreshRow:row];
    }
}

- (void)applicationChosen:(id)sender
{
    NSInteger index = [_applicationPopup indexOfSelectedItem];
    if (index <= 0) {
        [_applicationField setStringValue:@""];
        [self selectApplicationDomain:nil];
        return;
    }
    NSString *identifier = _installedApplications[index - 1][@"identifier"];
    [_applicationField setStringValue:identifier];
    [self selectApplicationDomain:identifier];
}

#pragma mark Showing values

- (id)pendingValueForRow:(DPRow *)row found:(BOOL *)found
{
    NSArray *change = _pending[row.section.identifier][row.setting.key];
    *found = change != nil;
    id value = change[1];
    return value == [NSNull null] ? nil : value;
}

- (id)valueShownForRow:(DPRow *)row
{
    if (row.setting.scope == DPScopeSystem) {
        BOOL pending;
        id value = [self pendingValueForRow:row found:&pending];
        if (pending)
            return value;
    }
    NSString *domain = row.perApplication ? _applicationDomain : row.setting.domain;
    if (domain == nil)
        return nil;
    id value = [row.setting storedValueInDomain:domain];
    if (value == nil && [row.setting.key isEqualToString:@"DarlingMenuBarStyle"]) {
        // the older boolean DarlingInWindowMenuBar means "window" while no style is set
        DPSetting *legacy = [DPSetting settingWithKey:@"DarlingInWindowMenuBar" domain:row.setting.domain kind:DPKindBool
                                                scope:DPScopeUser title:nil help:nil defaultText:nil];
        id old = [legacy storedValueInDomain:domain];
        if ([legacy popupIndexForValue:old] == 1)
            return @"window";
    }
    return value;
}

- (void)refreshRow:(DPRow *)row
{
    id value = [self valueShownForRow:row];
    DPSetting *setting = row.setting;
    if ([row.control isKindOfClass:[NSPopUpButton class]])
        [(NSPopUpButton *)row.control selectItemAtIndex:[setting popupIndexForValue:value]];
    else
        [row.control setStringValue:[setting textForValue:value]];
}

- (void)refreshAll
{
    for (DPRow *row in _rows)
        [self refreshRow:row];
    for (NSString *identifier in _applyButtons) {
        BOOL dirty = [_pending[identifier] count] != 0;
        [_applyButtons[identifier] setEnabled:dirty];
        [_revertButtons[identifier] setEnabled:dirty];
    }
}

- (void)say:(NSString *)text error:(BOOL)error
{
    [_status setStringValue:text ?: @""];
    [_status setTextColor:error ? [NSColor redColor] : [NSColor grayColor]];
}

#pragma mark Changing values

static BOOL ValuesEqual(id a, id b)
{
    return a == b || (a != nil && b != nil && [a isEqual:b]);
}

- (void)userChangedRow:(DPRow *)row toValue:(id)value
{
    DPSetting *setting = row.setting;
    if (setting.scope == DPScopeSystem) {
        // edited, written by Apply; a value that equals what is stored is no change
        NSMutableDictionary *changes = _pending[row.section.identifier];
        id stored = [setting storedValueInDomain:nil];
        if (ValuesEqual(value, stored))
            [changes removeObjectForKey:setting.key];
        else
            changes[setting.key] = @[ setting, value ?: [NSNull null] ];
        BOOL dirty = [changes count] != 0;
        [_applyButtons[row.section.identifier] setEnabled:dirty];
        [_revertButtons[row.section.identifier] setEnabled:dirty];
        [self say:dirty ? NSLocalizedString(@"Click Apply to write the changes.", nil) : @"" error:NO];
        return;
    }

    NSString *domain = row.perApplication ? _applicationDomain : setting.domain;
    if (domain == nil)
        return;
    BOOL written = [setting writeUserValue:value toDomain:domain];
    if ([setting.key isEqualToString:@"DarlingMenuBarStyle"]) {
        // a choice replaces the older boolean, which would otherwise keep acting as "window"
        DPSetting *legacy = [DPSetting settingWithKey:@"DarlingInWindowMenuBar" domain:setting.domain kind:DPKindBool
                                                scope:DPScopeUser title:nil help:nil defaultText:nil];
        written = [legacy writeUserValue:nil toDomain:domain] && written;
    }
    [self refreshRow:row];
    if (written)
        [self say:[NSString stringWithFormat:NSLocalizedString(@"%@ saved. It applies to applications started from now on.", nil), setting.title] error:NO];
    else
        [self say:[NSString stringWithFormat:NSLocalizedString(@"%@ could not be saved.", nil), setting.title] error:YES];
}

- (DPRow *)rowOfControl:(id)control
{
    NSInteger tag = [control tag];
    return tag >= 0 && tag < (NSInteger)[_rows count] ? _rows[tag] : nil;
}

- (void)popupChanged:(NSPopUpButton *)sender
{
    DPRow *row = [self rowOfControl:sender];
    if (row == nil)
        return;
    [self userChangedRow:row toValue:[row.setting valueForPopupIndex:[sender indexOfSelectedItem]]];
}

- (void)controlTextDidEndEditing:(NSNotification *)notification
{
    NSTextField *field = [notification object];
    if (field == _applicationField) {
        NSString *typed = [field stringValue];
        NSString *identifier = [typed length] != 0 ? [self validBundleIdentifier:typed] : nil;
        if ([typed length] != 0 && identifier == nil) {
            [self say:NSLocalizedString(@"A bundle identifier has letters, digits, dots, dashes and underscores only.", nil) error:YES];
            return;
        }
        [_applicationPopup selectItemAtIndex:0];
        for (NSUInteger i = 0; i < [_installedApplications count]; i++)
            if ([_installedApplications[i][@"identifier"] isEqualToString:identifier])
                [_applicationPopup selectItemAtIndex:i + 1];
        [self selectApplicationDomain:identifier];
        return;
    }

    DPRow *row = [self rowOfControl:field];
    if (row == nil)
        return;
    NSString *problem = nil;
    id value = [row.setting valueForText:[field stringValue] error:&problem];
    if (problem != nil) {
        [self say:[NSString stringWithFormat:@"%@: %@", row.setting.title, problem] error:YES];
        [self refreshRow:row];
        return;
    }
    if (!ValuesEqual(value, [self valueShownForRow:row]))
        [self userChangedRow:row toValue:value];
    [self refreshRow:row];
}

#pragma mark Apply and Revert

- (NSString *)sectionIdentifierOfButton:(NSButton *)button
{
    return [button toolTip];
}

- (id)effectiveValueOfKey:(NSString *)key inSection:(DPSection *)section
{
    for (DPSetting *setting in section.systemSettings) {
        if (![setting.key isEqualToString:key])
            continue;
        NSArray *change = _pending[section.identifier][key];
        if (change != nil)
            return change[1] == [NSNull null] ? nil : change[1];
        return [setting storedValueInDomain:nil];
    }
    return nil;
}

- (void)apply:(NSButton *)sender
{
    NSString *identifier = [self sectionIdentifierOfButton:sender];
    DPSection *section = nil;
    for (DPSection *candidate in [DPSection allSections])
        if ([candidate.identifier isEqualToString:identifier])
            section = candidate;
    NSDictionary *changes = _pending[identifier];
    if (section == nil || [changes count] == 0)
        return;

    // a field that is still being edited gives up its text first
    [[[self mainView] window] makeFirstResponder:[[self mainView] window]];

    // darling-prefsd refuses the whole domain when the two differ
    NSString *platform = [self effectiveValueOfKey:@"PlatformUUID" inSection:section];
    NSString *udid = [self effectiveValueOfKey:@"ProvisioningUDID" inSection:section];
    if (platform != nil && udid != nil && [platform caseInsensitiveCompare:udid] != NSOrderedSame) {
        [self say:NSLocalizedString(@"The provisioning UDID must equal the platform UUID. Nothing was written.", nil) error:YES];
        return;
    }

    NSMutableArray *ordered = [NSMutableArray array];
    for (DPSetting *setting in section.systemSettings)
        if (changes[setting.key] != nil)
            [ordered addObject:changes[setting.key]];

    NSString *failure = nil;
    BOOL success = [DPSetting applySystemChanges:ordered error:&failure];
    // what was written is no longer pending
    NSMutableDictionary *remaining = _pending[identifier];
    for (DPSetting *setting in section.systemSettings) {
        NSArray *change = remaining[setting.key];
        if (change != nil && ValuesEqual(change[1] == [NSNull null] ? nil : change[1], [setting storedValueInDomain:nil]))
            [remaining removeObjectForKey:setting.key];
    }
    [self refreshAll];
    if (success)
        [self say:NSLocalizedString(@"Applied.", nil) error:NO];
    else
        [self say:failure error:YES];
}

- (void)revert:(NSButton *)sender
{
    [_pending[[self sectionIdentifierOfButton:sender]] removeAllObjects];
    [self refreshAll];
    [self say:@"" error:NO];
}

#pragma mark Pane

- (NSView *)loadMainView
{
    _rows = [NSMutableArray array];
    _contentHeights = [NSMutableArray array];
    _pending = [NSMutableDictionary dictionary];
    _applyButtons = [NSMutableDictionary dictionary];
    _revertButtons = [NSMutableDictionary dictionary];

    NSView *main = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, kPaneWidth, 400)];
    _tabs = [[NSTabView alloc] initWithFrame:NSMakeRect(10, kStatusHeight, kPaneWidth - 20, 370)];
    [_tabs setDelegate:self];
    for (DPSection *section in [DPSection allSections]) {
        _pending[section.identifier] = [NSMutableDictionary dictionary];
        NSTabViewItem *item = [[NSTabViewItem alloc] initWithIdentifier:section.identifier];
        [item setLabel:section.title];
        NSView *content = [self viewForSection:section];
        [item setView:content];
        [_tabs addTabViewItem:item];
        [_contentHeights addObject:@([content frame].size.height)];
    }
    [_tabs setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [main addSubview:_tabs];

    _status = MakeLabel(@"", NSMakeRect(kMargin, 6, kPaneWidth - 2 * kMargin, 18),
                        [NSFont systemFontOfSize:[NSFont smallSystemFontSize]], [NSColor grayColor], NSTextAlignmentLeft);
    [_status setAutoresizingMask:NSViewWidthSizable | NSViewMaxYMargin];
    [main addSubview:_status];

    [self setMainView:main];
    [self refreshAll];
    [self fitToSelectedTab];
    [self mainViewDidLoad];
    return main;
}

- (void)willSelect
{
    [self refreshAll];
}

// The pane is as tall as the selected tab needs; the host follows when this changes.
- (void)fitToSelectedTab
{
    NSInteger index = [_tabs indexOfTabViewItem:[_tabs selectedTabViewItem]];
    if (index < 0 || index >= (NSInteger)[_contentHeights count])
        return;
    CGFloat content = [_contentHeights[index] doubleValue];
    // what the tab view adds around the content: the tabs and the border
    NSRect frame = [_tabs frame];
    CGFloat overhead = frame.size.height - [_tabs contentRect].size.height;
    if (overhead < 20 || overhead > 80)
        overhead = 46;
    CGFloat height = MAX(content + overhead, 200) + kStatusHeight + 10;
    NSView *main = [self mainView];
    if (fabs([main frame].size.height - height) > 0.5)
        [main setFrameSize:NSMakeSize(kPaneWidth, height)];
}

- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)item
{
    [self say:@"" error:NO];
    [self fitToSelectedTab];
}

#pragma mark Anchors

// An anchor names a tab (as its identifier or title) or the key of a setting in it.
- (void)handleSystemPreferencesAnchor:(NSString *)anchor
{
    NSString *wanted = [[anchor stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] lowercaseString];
    if ([wanted length] == 0 || _tabs == nil)
        return;
    NSMutableDictionary *aliases = [NSMutableDictionary dictionary];
    aliases[@"application lifecycle"] = @"applications";
    aliases[@"keyboard and input"] = @"keyboard";
    aliases[@"display and windows"] = @"display";
    aliases[@"file dialogs"] = @"files";
    aliases[@"platform identity"] = @"identity";
    aliases[@"metal diagnostics"] = @"metal";
    NSString *target = aliases[wanted];
    for (DPSection *section in [DPSection allSections]) {
        if ([[section.identifier lowercaseString] isEqualToString:wanted] || [[section.title lowercaseString] isEqualToString:wanted])
            target = section.identifier;
        for (DPSetting *setting in [section.userSettings arrayByAddingObjectsFromArray:section.systemSettings ?: @[]]) {
            if (target == nil && [[setting.key lowercaseString] isEqualToString:wanted])
                target = section.identifier;
        }
    }
    if (target == nil)
        return;
    NSInteger index = [_tabs indexOfTabViewItemWithIdentifier:target];
    if (index != NSNotFound) {
        [_tabs selectTabViewItemAtIndex:index];
        [self fitToSelectedTab];
    }
}

@end
