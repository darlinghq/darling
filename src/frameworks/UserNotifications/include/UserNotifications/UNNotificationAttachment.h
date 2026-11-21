/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel
 Copyright (C) 2026 Darling developers

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

#ifndef _USERNOTIFICATIONS_UNNOTIFICATIONATTACHMENT_H_
#define _USERNOTIFICATIONS_UNNOTIFICATIONATTACHMENT_H_

#import <Foundation/Foundation.h>

// Option keys of +attachmentWithIdentifier:URL:options:error:. The value of the type hint is
// a uniform type identifier string.
extern NSString *const UNNotificationAttachmentOptionsTypeHintKey;

@interface UNNotificationAttachment : NSObject <NSCopying, NSSecureCoding> {
    NSString *_identifier;
    NSURL *_URL;
    NSString *_type;
}

+ (nullable instancetype)attachmentWithIdentifier:(nullable NSString *)identifier
                                              URL:(NSURL *)URL
                                          options:(nullable NSDictionary *)options
                                            error:(NSError **)error;

@property(readonly, copy) NSString *identifier;
@property(readonly, copy) NSURL *URL;
@property(readonly, copy) NSString *type;

@end

#endif
