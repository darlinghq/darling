/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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

#ifndef CONTACTS_CNLABELEDVALUE_H
#define CONTACTS_CNLABELEDVALUE_H

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// A contact property value (a phone number, an email address, ...) together with an optional label such as
// "home" or "work". Instances are immutable: the labeledValueBySetting... methods return new objects.
API_AVAILABLE(macos(10.11))
@interface CNLabeledValue<ValueType : id <NSCopying, NSSecureCoding>> : NSObject <NSCopying, NSSecureCoding>

// Unique among the labeled values of a device, so it can be used to find a previously known labeled value.
@property (readonly, copy, NS_NONATOMIC_IOSONLY) NSString *identifier;
@property (readonly, nullable, copy, NS_NONATOMIC_IOSONLY) NSString *label;
@property (readonly, copy, NS_NONATOMIC_IOSONLY) ValueType value;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

+ (instancetype)labeledValueWithLabel:(nullable NSString *)label value:(ValueType)value;
- (instancetype)initWithLabel:(nullable NSString *)label value:(ValueType)value;

// The new object keeps the identifier of the receiver.
- (instancetype)labeledValueBySettingLabel:(nullable NSString *)label;
- (instancetype)labeledValueBySettingValue:(ValueType)value;
- (instancetype)labeledValueBySettingLabel:(nullable NSString *)label value:(ValueType)value;

+ (NSString *)localizedStringForLabel:(NSString *)label;

@end

NS_ASSUME_NONNULL_END

#endif
