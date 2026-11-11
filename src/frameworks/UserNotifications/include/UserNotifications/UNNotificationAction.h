/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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

#ifndef _USERNOTIFICATIONS_UNNOTIFICATIONACTION_H_
#define _USERNOTIFICATIONS_UNNOTIFICATIONACTION_H_

#import <Foundation/Foundation.h>

typedef NS_OPTIONS(NSUInteger, UNNotificationActionOptions) {
    UNNotificationActionOptionNone = 0,
    UNNotificationActionOptionAuthenticationRequired = (1 << 0),
    UNNotificationActionOptionDestructive = (1 << 1),
    UNNotificationActionOptionForeground = (1 << 2)
};

@interface UNNotificationAction : NSObject <NSCopying, NSSecureCoding> {
    NSString *_identifier;
    NSString *_title;
    UNNotificationActionOptions _options;
}

+ (instancetype) actionWithIdentifier: (NSString *) identifier
                                title: (NSString *) title
                              options: (UNNotificationActionOptions) options;

@property(readonly, copy) NSString *identifier;
@property(readonly, copy) NSString *title;
@property(readonly) UNNotificationActionOptions options;

@end

#endif
