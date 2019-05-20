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

- (instancetype)initWithLabel:(NSString *)label value:(ValueType)value;
+ (instancetype)labeledValueWithLabel:(NSString *)label value:(ValueType)value;

@property(readonly, copy, atomic) NSString *label;
@property(readonly, copy, atomic) ValueType value;

- (instancetype)labeledValueBySettingLabel:(NSString *)label;
- (instancetype)labeledValueBySettingLabel:(NSString *)label value:(ValueType)value;
- (instancetype)labeledValueBySettingValue:(ValueType)value;

+ (NSString *)localizedStringForLabel:(NSString *)label;

@property(readonly, copy, atomic) NSString *identifier;

@end
