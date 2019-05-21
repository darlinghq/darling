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
    if (self = [super init]) {
//        [self applyLabeledValueVariablesIdentifier: [[NSUUID alloc] init].UUIDString label:label value:value];
        [self applyLabeledValueVariablesIdentifier: [[NSUUID alloc] init].UUIDString label:label value:value];
    }
    
    return self;
}

- (instancetype)initWithIdentifier:(NSString *)identifier label:(NSString *)label value:(id)value {
    if (self = [super init]) {
        [self applyLabeledValueVariablesIdentifier:identifier label:label value:value];
    }
    
    return self;
}

- (void)dealloc {
    [_identifier release];
    [_label release];
    [_value release];
    [super dealloc];
}

- (instancetype)applyLabeledValueVariablesIdentifier:(NSString *)identifier label:(NSString *)label value:(id)value {
    if (value == nil) {
        @throw [[NSException alloc] initWithName:@"CNPropertyInvalidTypeExpression" reason:@"Variable value cannot be null." userInfo:nil];
    }
    
    _identifier = [identifier copy];
    _label = [label copy];
    _value = [value copy];
    return self;
}

+ (instancetype)labeledValueWithLabel:(NSString *)label value:(id)value {
    return [[[[self class] alloc] initWithLabel:label value:value] autorelease];
}

- (instancetype)labeledValueBySettingLabel:(NSString *)label {
    return [[[[self class] alloc] initWithIdentifier:_identifier label:label value:_value] autorelease];
}

- (instancetype)labeledValueBySettingLabel:(NSString *)label value:(id)value {
    return [[[[self class] alloc] initWithIdentifier:_identifier label:label value:value] autorelease];
}

- (instancetype)labeledValueBySettingValue:(id)value {
    return [[[[self class] alloc] initWithIdentifier:_identifier label:_label value:value] autorelease];
}

/*
 TODO: Add support for obtaining localized string from contact label
 */
+ (NSString *)localizedStringForLabel:(NSString *)label {
    // If you provide a custom label, it will return the same label.
    return label;
}


// NSCopy
- (id)copyWithZone:(NSZone *)zone {
    id copy = [[[self class] alloc] init];
    return [copy autorelease];
}


// NSSecureCoder
+ (BOOL)supportsSecureCoding {
    return YES;
}

- (void)encodeWithCoder:(NSCoder *)aCoder {
    return;
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
    return self;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
