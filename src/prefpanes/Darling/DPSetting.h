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

#import <Foundation/Foundation.h>

// One Darling setting: a key of a defaults domain, what kind of value it holds, how it is
// shown, and how it is read and written.
//
// A setting has a scope:
//  - DPScopeUser: a value in the user's domain (`defaults write <domain> <key>`), read and
//    written through CFPreferences' application values, so it applies to the user's
//    applications started afterwards (or to the domain of one application, see the domain
//    argument of the read and write methods).
//  - DPScopeSystem: a value in the system-wide domain, /Library/Preferences/<domain>.plist
//    (kCFPreferencesAnyUser, kCFPreferencesCurrentHost), where darling-prefsd and the
//    components that read these keys look for it. A change is written with
//    +applySystemChanges:error:.
//
// The values are property list objects: NSNumber (boolean, integer or real), NSString or
// NSData. A nil value means "the key is not set", the documented default.
typedef NS_ENUM(NSInteger, DPKind) {
    DPKindBool,      // On / Off
    DPKindChoice,    // one of a list of strings
    DPKindText,      // a string; stringLimit characters at most when it is not 0, printable ASCII when asciiOnly
    DPKindInteger,   // a positive integer
    DPKindReal,      // a number of at least minimum
    DPKindUUID,      // a UUID string, not the nil UUID
    DPKindHex,       // data of byteLength bytes, typed in hexadecimal, not all zero
    DPKindPath       // an absolute path
};

typedef NS_ENUM(NSInteger, DPScope) {
    DPScopeUser,
    DPScopeSystem
};

@interface DPSetting : NSObject

@property (copy) NSString *key;
@property (copy) NSString *domain;
@property DPKind kind;
@property DPScope scope;
@property (copy) NSString *title;
@property (copy) NSString *help;
// What applies while the key is not set, as the popup's first item reads it ("Off", "Automatic").
@property (copy) NSString *defaultText;
// DPKindBool, DPKindChoice: the values and their titles (a boolean has none).
@property (copy) NSArray<NSString *> *choices;
@property (copy) NSArray<NSString *> *choiceTitles;
@property (copy) NSString *placeholder;
@property NSUInteger stringLimit;      // text and path: the longest accepted, in bytes
@property BOOL asciiOnly;
@property NSUInteger byteLength;       // hex
@property double minimum;              // real and integer: the smallest accepted number (shown, not stored)
@property double scale;                // real and integer: the stored number is the typed one times scale (1 when 0)
@property (copy) NSString *unit;       // shown after a number ("GiB", "MB")

+ (instancetype)settingWithKey:(NSString *)key domain:(NSString *)domain kind:(DPKind)kind scope:(DPScope)scope
                         title:(NSString *)title help:(NSString *)help defaultText:(NSString *)defaultText;

// The value stored for the setting in the domain (the setting's own when the argument is nil),
// nil when the key is not set. For the user scope the lookup is CFPreferencesCopyAppValue, which
// finds a system-wide value as well when the user has none.
- (id)storedValueInDomain:(NSString *)domain;

// Text to put in a field for a stored value, and the value a typed text means. nil text or an
// empty one is the default (nil value, error stays nil). For text that is not acceptable
// the result is nil and *error says why.
- (NSString *)textForValue:(id)value;
- (id)valueForText:(NSString *)text error:(NSString **)error;

// For booleans and choices: the index in a popup (0 is the default, then the choices) and back.
- (NSInteger)popupIndexForValue:(id)value;
- (id)valueForPopupIndex:(NSInteger)index;
- (NSArray<NSString *> *)popupTitles;

// Writes (nil removes) a user-scope value into the domain, for the current user, and
// synchronizes it. Returns NO when the preferences could not be written.
- (BOOL)writeUserValue:(id)value toDomain:(NSString *)domain;

// System scope: writes every change (a pair of the setting and its value, an NSNull removing the
// key) to the system-wide domain and synchronizes it. Stops at the first change that cannot be
// written and describes it in *error; the changes before it stay written.
+ (BOOL)applySystemChanges:(NSArray<NSArray *> *)changes error:(NSString **)error;

@end
