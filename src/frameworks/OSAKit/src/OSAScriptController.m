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

#import <OSAKit/OSAScriptController.h>
#import <darling-stub-forwarding.h>

@implementation OSAScriptController

@synthesize scriptView = _scriptView;
@synthesize resultView = _resultView;
@synthesize script = _script;
@synthesize language = _language;
@synthesize scriptState = _scriptState;
@synthesize compiling = _compiling;
@synthesize undoManager = _undoManager;
@synthesize defaultTarget = _defaultTarget;

// Nib outlets arrive through connectors; NSController's own coder support is unimplemented.
- (instancetype)initWithCoder:(NSCoder *)coder
{
    return [super init];
}

- (void)dealloc
{
    [_script release];
    [_language release];
    [_undoManager release];
    [_defaultTarget release];
    [super dealloc];
}

- (OSALanguage *)language
{
    return _language ?: [_script language];
}

- (void)setLanguage:(OSALanguage *)language
{
    [language retain];
    [_language release];
    _language = language;
}

- (void)showError:(NSDictionary *)errorInfo
{
    NSString *message = [errorInfo objectForKey:OSAScriptErrorMessageKey];
    if (message != nil)
        [_resultView setString:message];
}

- (IBAction)compileScript:(id)sender
{
    OSAScript *script = [[[OSAScript alloc] initWithSource:[_scriptView source] ?: @"" language:[self language]] autorelease];
    NSDictionary *errorInfo = nil;
    [self setIsCompiling:YES];
    BOOL compiled = [script compileAndReturnError:&errorInfo];
    [self setIsCompiling:NO];
    [self setScript:script];
    if (!compiled)
        [self showError:errorInfo];
}

- (IBAction)runScript:(id)sender
{
    [self compileScript:sender];
    if (![_script isCompiled])
        return;

    NSAttributedString *displayValue = nil;
    NSDictionary *errorInfo = nil;
    _scriptState = OSAScriptRunning;
    NSAppleEventDescriptor *result = [_script executeAndReturnDisplayValue:&displayValue error:&errorInfo];
    _scriptState = OSAScriptStopped;
    if (result == nil)
        [self showError:errorInfo];
    else if (displayValue != nil)
        [[_resultView textStorage] setAttributedString:displayValue];
}

// There is no Apple event recorder, and scripts run synchronously, so neither has anything to do.
- (IBAction)recordScript:(id)sender
{
}

- (IBAction)stopScript:(id)sender
{
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
