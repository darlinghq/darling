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

API_UNAVAILABLE_BEGIN(ios, tvos, watchos)

// What shouldUnselect answers: keep the pane selected, leave now, or leave
// once the pane has called replyToShouldUnselect:.
typedef NS_ENUM(NSUInteger, NSPreferencePaneUnselectReply)
{
    NSUnselectCancel = 0,
    NSUnselectNow = 1,
    NSUnselectLater = 2
};

// Posted by -replyToShouldUnselect: to tell the host application whether the
// pane agreed (DoUnselect) or refused (CancelUnselect) to be unselected.
extern NSString * __nonnull const NSPreferencePaneDoUnselectNotification;
extern NSString * __nonnull const NSPreferencePaneCancelUnselectNotification;

// Posted to ask the host application to show another pane; the host defines
// what the notification carries.
extern NSString * __nonnull const NSPreferencePaneSwitchToPaneNotification;
extern NSString * __nonnull const NSPreferencePrefPaneIsAvailableNotification;
// Posted by -updateHelpMenuWithArray:.
extern NSString * __nonnull const NSPreferencePaneUpdateHelpMenuNotification;

// Help menu support: the Info.plist key that lists static help items, and
// the keys of each item dictionary.
extern NSString * __nonnull const NSPrefPaneHelpMenuInfoPListKey;
extern NSString * __nonnull const NSPrefPaneHelpMenuTitleKey;
extern NSString * __nonnull const NSPrefPaneHelpMenuAnchorKey;

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7

#define kNSPrefPaneHelpMenuInfoPListKey NSPrefPaneHelpMenuInfoPListKey
#define kNSPrefPaneHelpMenuTitleKey     NSPrefPaneHelpMenuTitleKey
#define kNSPrefPaneHelpMenuAnchorKey    NSPrefPaneHelpMenuAnchorKey

#else /* MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_7 */

#define kNSPrefPaneHelpMenuInfoPListKey @"NSPrefPaneHelpAnchors"
#define kNSPrefPaneHelpMenuTitleKey     @"title"
#define kNSPrefPaneHelpMenuAnchorKey    @"anchor"

#endif

@interface NSPreferencePane : NSObject
{
    @private
        // Outlets for the main nib file: the window whose content view
        // becomes the main view, and the three key views of the focus chain.
        IBOutlet NSWindow * __nullable  _window;
        IBOutlet NSView * __nullable    _initialKeyView;
        IBOutlet NSView * __nullable    _firstKeyView;
        IBOutlet NSView * __nullable    _lastKeyView;

        NSView * __nonnull      _mainView;
        NSBundle * __nonnull    _bundle;

        __nullable id _reserved1;
        __nullable id _reserved2;
        __nullable id _reserved3;
}

- (nonnull instancetype)initWithBundle:(nonnull NSBundle *)bundle;

@property (readonly, strong, nonnull) NSBundle *bundle;

// Loads the main nib file (see mainNibName) with the pane as its owner,
// calls assignMainView, then mainViewDidLoad, and returns the main view.
- (nonnull NSView *)loadMainView;

// Called once the nib has been loaded and the main view assigned.
- (void)mainViewDidLoad;

// The nib's name without its extension; by default the bundle's
// NSMainNibFile value.
@property (readonly, strong, nonnull) NSString *mainNibName;

// Takes the content view of the _window outlet as the main view.
- (void)assignMainView;

// Selection notifications.
- (void)willSelect;
- (void)didSelect;

// The host asks before it switches away from the pane. The default answer is
// NSUnselectNow. A pane that answers NSUnselectLater must follow up with
// replyToShouldUnselect:.
@property (readonly) NSPreferencePaneUnselectReply shouldUnselect;
- (void)replyToShouldUnselect:(BOOL)shouldUnselect;

- (void)willUnselect;
- (void)didUnselect;

@property (strong, nonnull) NSView *mainView;

// The key views of the pane; by default the outlets above.
@property (strong, nullable) NSView *initialKeyView;
@property (strong, nullable) NSView *firstKeyView;
@property (strong, nullable) NSView *lastKeyView;

// Default YES: the host makes text fields end editing before it asks
// shouldUnselect.
@property (readonly) BOOL autoSaveTextFields;

// YES while the user has the pane selected.
@property (getter=isSelected, readonly) BOOL selected;

// Replaces the dynamic part of the help menu. Each item is a dictionary
// with a localized title and an anchor for AHLookupAnchor.
- (void)updateHelpMenuWithArray:(nullable NSArray<NSDictionary<NSString *, NSString *> *> *)inArrayOfMenuItems;
@end

API_UNAVAILABLE_END
