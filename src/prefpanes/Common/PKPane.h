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

// The shared base of the standard preference panes (General, Language & Region, Keyboard, Date & Time).
// A pane lists its settings as PKItems; the base builds the controls, shows what the preferences hold,
// and writes a change through CFPreferences when the user operates a control, then lets the pane react
// and posts the distributed notification that announces the change. Every control's tool tip is the
// item's identifier, which is how tests find it.
typedef NS_ENUM(NSInteger, PKKind) {
    PKKindHeading,   // a bold title above the items that follow
    PKKindNote,      // a paragraph of explanation
    PKKindPopup,     // a pop-up menu: values[i] is stored for the entry titles[i]
    PKKindCheck,     // a check box for a boolean
    PKKindSlider,    // a slider with one tick per entry of values; titles has the labels of its two ends
    PKKindSwatches,  // a row of round colour buttons: values[i] is stored for the colour colors[i]
    PKKindStatic,    // a label whose text a pane supplies (staticText)
    PKKindView       // a view the pane builds (view); it is placed at the left margin
};

@interface PKItem : NSObject
@property (copy) NSString *identifier;
@property (copy) NSString *title;          // the label at the left (the check box's own title for a check box)
@property (copy) NSString *help;           // grey explanation under the control
@property PKKind kind;
// The preference: the key in the domain (nil means NSGlobalDomain), for this host only when currentHost.
@property (copy) NSString *key;
@property (copy) NSString *domain;
@property BOOL currentHost;
// The value that applies while the key is not set. A pop-up entry whose stored value is NSNull
// removes the key.
@property (strong) id defaultValue;
@property (copy) NSArray *values;
@property (copy) NSArray<NSString *> *titles;
@property (copy) NSArray<NSColor *> *colors;
@property BOOL inverted;                   // a check box that is on when the boolean is NO
@property (copy) NSString *notification;   // the distributed notification posted after a change
@property (copy) NSString *(^staticText)(void);
@property (strong) NSView *view;
// For a setting that is not one plain key: how its value is read and written instead (nil removes)
@property (copy) id (^readBlock)(void);
@property (copy) void (^writeBlock)(id value);

+ (instancetype)heading:(NSString *)title;
+ (instancetype)note:(NSString *)text;
+ (instancetype)popup:(NSString *)identifier title:(NSString *)title key:(NSString *)key values:(NSArray *)values titles:(NSArray *)titles defaultValue:(id)defaultValue;
+ (instancetype)check:(NSString *)identifier title:(NSString *)title key:(NSString *)key defaultValue:(BOOL)defaultValue;
+ (instancetype)slider:(NSString *)identifier title:(NSString *)title key:(NSString *)key values:(NSArray *)values labels:(NSArray *)labels defaultValue:(id)defaultValue;
+ (instancetype)staticText:(NSString *)identifier title:(NSString *)title text:(NSString *(^)(void))text;

// The value in the preferences, nil when the key is not set; and writing one (nil removes the key).
- (id)storedValue;
- (void)storeValue:(id)value;
@end

@interface PKPane : NSPreferencePane
// What a pane provides
- (NSArray<PKItem *> *)items;
// Called after the item's value was written, before the notification is posted.
- (void)itemDidChange:(PKItem *)item toValue:(id)value;
// Called while the pane builds and refreshes its view, to refresh what the pane draws itself.
- (void)refreshCustomViews;
// The width of the pane's view, and the room above the first item
+ (CGFloat)paneWidth;

// For the panes
- (NSControl *)controlForIdentifier:(NSString *)identifier;
- (void)refresh;
// Updates only the text of the static items (a clock), leaving the other controls alone
- (void)refreshStaticItems;
// Writes a value of a preference that is not an item (nil removes it), as the items are written.
+ (void)setPreference:(id)value forKey:(NSString *)key domain:(NSString *)domain currentHost:(BOOL)currentHost;
+ (id)preferenceForKey:(NSString *)key domain:(NSString *)domain currentHost:(BOOL)currentHost;
+ (void)postNotification:(NSString *)name;
@end
