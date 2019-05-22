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

#define NSCODER_IDENTIFIER @"_identifier"
#define NSCODER_LABEL @"_label"
#define NSCODER_VALUE @"_value"

const NSExceptionName CNPropertyInvalidTypeExceptionName = @"CNPropertyInvalidTypeException";

@implementation CNLabeledValue

@synthesize identifier = _identifier;
@synthesize label = _label;
@synthesize value = _value;

- (instancetype)initWithLabel:(NSString *)label value:(id)value {
    NSUUID *uuid = [NSUUID UUID];
    return [self initWithIdentifier: [uuid UUIDString] label:label value:value];
}

- (void)dealloc {
    [_identifier release];
    [_label release];
    [_value release];
    [super dealloc];
}

+ (instancetype)labeledValueWithLabel:(NSString *)label value:(id)value {
    return [[[self alloc] initWithLabel:label value:value] autorelease];
}

- (instancetype)labeledValueBySettingLabel:(NSString *)label {
    return [[[CNLabeledValue alloc] initWithIdentifier:_identifier label:label value:_value] autorelease];
}

- (instancetype)labeledValueBySettingLabel:(NSString *)label value:(id)value {
    return [[[CNLabeledValue alloc] initWithIdentifier:_identifier label:label value:value] autorelease];
}

- (instancetype)labeledValueBySettingValue:(id)value {
    return [[[CNLabeledValue alloc] initWithIdentifier:_identifier label:_label value:value] autorelease];
}

/*
 TODO: Add support for obtaining localized string from contact label
 */
+ (NSString *)localizedStringForLabel:(NSString *)label {
    // If you provide a custom label, it will return the same label.
    return label;
}


// Private method.
- (instancetype)initWithIdentifier:(NSString *)identifier
                             label:(NSString *)label
                             value:(id)value {
    if (self = [super init]) {
        if (value == nil) {
            [NSException raise:CNPropertyInvalidTypeExceptionName format:@"Variable value cannot be null."];
        }
        
        _identifier = [identifier copy];
        _label = [label copy];
        _value = [value copy];
    }
    
    return self;
}

// NSCopying
- (id)copyWithZone:(NSZone *)zone {
    id copy = [[[self class] alloc] initWithIdentifier:_identifier label:_label value:_value];
    return copy;
}


// NSSecureCoder
+ (BOOL)supportsSecureCoding {
    return YES;
}

/*
 When you look at the original CNLabeledValue .plist, there are also
 three other keynames: linkedIdentifiers, storeIdentifier, storeInfo.
 
 TODO: Figure out what they are used for */
- (void)encodeWithCoder:(NSCoder *)aCoder {
    [aCoder encodeObject:_identifier forKey:NSCODER_IDENTIFIER];
    [aCoder encodeObject:_label forKey:NSCODER_LABEL];
    [aCoder encodeObject:_value forKey:NSCODER_VALUE];
    
    return;
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
    NSString *temp_identifier = [aDecoder decodeObjectOfClass:[NSString class] forKey:NSCODER_IDENTIFIER];
    NSString *temp_lable = [aDecoder decodeObjectOfClass:[NSString class] forKey:NSCODER_LABEL];
    NSString *temp_value = [aDecoder decodeObjectOfClass:[NSString class] forKey:NSCODER_VALUE];
    
    // Since initWithIdentifier already uses copy for the variable, I decided just call
    // that function instead.
    return [self initWithIdentifier:temp_identifier label:temp_lable value:temp_value];
}


- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
