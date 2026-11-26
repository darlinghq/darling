/*
 This file is part of Darling.

 Copyright (C) 2025-2026 Darling Developers

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

#import <UIFoundation/NSTextAlternatives.h>
#import "UIFoundation-ExtraIvars.h"

NSNotificationName const NSTextAlternativesSelectedAlternativeStringNotification = @"NSTextAlternativesSelectedAlternativeStringNotification";

// The key of the chosen string in the notification's userInfo. Not given by
// Apple's reference; our own.
static NSString *const kAlternativeStringKey = @"NSAlternativeString";

// NSTextAlternatives (Apple's NSTextAlternatives reference, macOS 10.8).
@implementation NSTextAlternatives

#define EXTRA UIF_EXTRA_NSTextAlternatives(self)

+ (BOOL) supportsSecureCoding {
    return YES;
}

- (instancetype) initWithPrimaryString: (NSString *) primaryString alternativeStrings: (NSArray<NSString *> *) alternativeStrings {
    self = [super init];
    if (self) {
        EXTRA->_primaryString = [primaryString copy];
        EXTRA->_alternativeStrings = [alternativeStrings copy];
    }
    return self;
}

- (instancetype) init {
    return [self initWithPrimaryString: @"" alternativeStrings: @[]];
}

- (void) dealloc {
    [EXTRA->_primaryString release];
    [EXTRA->_alternativeStrings release];
    [super dealloc];
}

- (NSString *) primaryString {
    return EXTRA->_primaryString;
}

- (NSArray<NSString *> *) alternativeStrings {
    return EXTRA->_alternativeStrings;
}

- (void) noteSelectedAlternativeString: (NSString *) alternativeString {
    NSDictionary *userInfo = alternativeString ? @{ kAlternativeStringKey: alternativeString } : nil;
    [[NSNotificationCenter defaultCenter] postNotificationName: NSTextAlternativesSelectedAlternativeStringNotification
                                                        object: self
                                                      userInfo: userInfo];
}

- (NSString *) description {
    return [NSString stringWithFormat: @"<%@: %p> %@ (%@)", [self class], self, EXTRA->_primaryString,
                                       [EXTRA->_alternativeStrings componentsJoinedByString: @", "]];
}

// The archive keys are Darling's own: an archive made on macOS may not decode
// here, nor the other way round.
- (void) encodeWithCoder: (NSCoder *) coder {
    [coder encodeObject: EXTRA->_primaryString forKey: @"NSPrimaryString"];
    [coder encodeObject: EXTRA->_alternativeStrings forKey: @"NSAlternativeStrings"];
}

- (instancetype) initWithCoder: (NSCoder *) coder {
    NSString *primary = [coder decodeObjectOfClass: [NSString class] forKey: @"NSPrimaryString"];
    NSSet *classes = [NSSet setWithObjects: [NSArray class], [NSString class], nil];
    NSArray *alternatives = [coder decodeObjectOfClasses: classes forKey: @"NSAlternativeStrings"];
    if (primary == nil || alternatives == nil) {
        [self release];
        return nil;
    }
    return [self initWithPrimaryString: primary alternativeStrings: alternatives];
}

@end
