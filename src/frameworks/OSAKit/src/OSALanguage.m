/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#import <OSAKit/OSALanguage.h>
#include <dispatch/dispatch.h>

// Darling has no OSA components. OSAKit reports a single AppleScript language so script editors can
// describe documents; its language instance and component calls stay stubs.
static const OSType kOSAScriptingComponentType = 'osa ';
static const OSType kAppleScriptSubtype = 'ascr';

@implementation OSALanguage

+ (OSALanguage *)appleScriptLanguage
{
    static OSALanguage *appleScript;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        appleScript = [[OSALanguage alloc] init];
    });
    return appleScript;
}

+ (NSArray *)availableLanguages
{
    return @[ [self appleScriptLanguage] ];
}

+ (OSALanguage *)defaultLanguage
{
    return [self appleScriptLanguage];
}

+ (void)setDefaultLanguage:(OSALanguage *)language
{
}

+ (OSALanguage *)languageForName:(NSString *)name
{
    return [name isEqualToString:@"AppleScript"] ? [self appleScriptLanguage] : nil;
}

+ (OSALanguage *)languageForSubType:(OSType)subType
{
    return subType == kAppleScriptSubtype ? [self appleScriptLanguage] : nil;
}

+ (OSALanguage *)languageForScriptDataDescriptor:(id)descriptor
{
    return [self appleScriptLanguage];
}

+ (OSALanguage *)languageForScript:(id)script
{
    return [self appleScriptLanguage];
}

- (NSString *)name
{
    return @"AppleScript";
}

- (NSString *)info
{
    return @"AppleScript";
}

- (NSString *)version
{
    return @"2.8";
}

- (OSType)type
{
    return kOSAScriptingComponentType;
}

- (OSType)subType
{
    return kAppleScriptSubtype;
}

- (OSType)manufacturer
{
    return 'aapl';
}

- (NSUInteger)features
{
    return 0;
}

- (BOOL)isThreadSafe
{
    return NO;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
