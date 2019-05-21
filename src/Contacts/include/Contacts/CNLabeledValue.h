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

#include <Foundation/Foundation.h>

@interface CNLabeledValue<__covariant ValueType> : NSObject <NSCopying,NSSecureCoding>

- (nonnull instancetype)initWithLabel:(nullable NSString *)label value:(nonnull ValueType)value;
+ (nonnull instancetype)labeledValueWithLabel:(nullable NSString *)label value:(nonnull ValueType)value;

@property(readonly, copy, atomic, nullable) NSString *label;
@property(readonly, copy, atomic, nonnull) ValueType value;

- (nonnull instancetype)labeledValueBySettingLabel:(nullable NSString *)label;
- (nonnull instancetype)labeledValueBySettingLabel:(nullable NSString *)label value:(nonnull ValueType)value;
- (nonnull instancetype)labeledValueBySettingValue:(nonnull ValueType)value;

+ (nonnull NSString *)localizedStringForLabel:(nonnull NSString *)label;

@property(readonly, copy, atomic, nonnull) NSString *identifier;

@end
