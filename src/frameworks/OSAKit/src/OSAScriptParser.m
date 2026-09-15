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

#import <OSAKit/OSAScriptParser.h>
#import <darling-stub-forwarding.h>
#import <OSAKit/OSAScript.h>
#import <OSAKit/OSAScriptElement.h>

@implementation OSAScriptParser

@synthesize element = _element;

+ (instancetype)parserWithScript:(OSAScript *)script
{
    OSAScriptParser *parser = [[[self alloc] init] autorelease];
    parser->_script = [script retain];
    return parser;
}

- (void)dealloc
{
    [_script release];
    [_element release];
    [super dealloc];
}

// There is no AppleScript parser: the result is one untitled root element spanning the source.
- (BOOL)parse
{
    [_element release];
    _element = [[OSAScriptElement alloc] initWithRange:NSMakeRange(0, [[_script source] length])];
    return YES;
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
