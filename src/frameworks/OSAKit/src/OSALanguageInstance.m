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

#import <OSAKit/OSALanguageInstance.h>
#import <darling-stub-forwarding.h>

// Darling has no OSA components, so there is never a component instance to hand out.
@implementation OSALanguageInstance {
    OSALanguage *_language;
}

+ (instancetype)languageInstanceWithLanguage:(OSALanguage *)language
{
    return [[[self alloc] initWithLanguage:language] autorelease];
}

+ (ComponentInstance)defaultAppleScriptComponentInstance
{
    return NULL;
}

- (instancetype)initWithLanguage:(OSALanguage *)language
{
    if (language == nil) {
        [self release];
        return nil;
    }
    self = [super init];
    if (self != nil)
        _language = [language retain];
    return self;
}

- (void)dealloc
{
    [_language release];
    [super dealloc];
}

- (OSALanguage *)language
{
    return _language;
}

- (ComponentInstance)componentInstance
{
    return NULL;
}

- (NSAttributedString *)richTextFromDescriptor:(NSAppleEventDescriptor *)descriptor
{
    NSString *text = [descriptor stringValue];
    if (text == nil)
        return nil;
    return [[[NSAttributedString alloc] initWithString:text] autorelease];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
