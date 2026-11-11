/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _RTTUtilities_H_
#define _RTTUtilities_H_

#import <Foundation/Foundation.h>

// Stub of the private real-time text (TTY/RTT) settings framework. Darling has no
// telephony, so TTY support is always off. Selectors come from the apps' references.

@interface RTTSettings : NSObject
+ (instancetype)sharedInstance;
- (BOOL)TTYSoftwareEnabled;
@end

// Returns the string for a TTY localization key; the stub returns the key itself.
NSString *ttyLocString(NSString *key);

#endif
