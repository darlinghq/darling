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

#import <Contacts/CNLabeledValue.h>

@implementation CNLabeledValue

- (instancetype)initWithLabel:(NSString *)label value:(id)value {
    return self;
}

+ (instancetype)labeledValueWithLabel:(NSString *)label value:(id)value {
    id new_class = [[[self class] alloc] init];
    return new_class;
}

- (instancetype)labeledValueBySettingLabel:(NSString *)label {
    return self;
}

- (instancetype)labeledValueBySettingLabel:(NSString *)label value:(id)value {
    return self;
}

- (instancetype)labeledValueBySettingValue:(id)value {
    return self;
}

+ (NSString *)localizedStringForLabel:(NSString *)label {
    return @"";
}


// NSCopy
- (nonnull id)copyWithZone:(nullable NSZone *)zone {
    id copy = [[[self class] alloc] init];
    return copy;
}


// NSSecureCoder
+ (BOOL)supportsSecureCoding {
    return YES;
}

- (void)encodeWithCoder:(nonnull NSCoder *)aCoder {
    return;
}

- (nullable instancetype)initWithCoder:(nonnull NSCoder *)aDecoder {
    return self;
}


- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
