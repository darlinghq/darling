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

#import <OSAKit/OSAScript.h>
#import "OSAKitPrivate.h"
#import <darling-stub-forwarding.h>

// Darling has no AppleScript component: scripts hold their source text, and anything that would need
// the engine (compiling, running, loading or producing compiled data) fails with an error dictionary.
static NSString *const kEngineUnavailableMessage = @"AppleScript is not available on Darling.";

static NSDictionary *errorInfoWith(NSString *message, NSInteger number)
{
    return [NSDictionary dictionaryWithObjectsAndKeys:
        message, OSAScriptErrorMessageKey,
        message, OSAScriptErrorBriefMessageKey,
        [NSNumber numberWithInteger:number], OSAScriptErrorNumberKey,
        [NSValue valueWithRange:NSMakeRange(0, 0)], OSAScriptErrorRangeKey,
        nil];
}

static void setErrorInfo(NSDictionary **errorInfo, NSString *message, NSInteger number)
{
    if (errorInfo != NULL)
        *errorInfo = errorInfoWith(message, number);
}

static void setEngineUnavailable(NSDictionary **errorInfo)
{
    setErrorInfo(errorInfo, kEngineUnavailableMessage, errOSACantOpenComponent);
}

static BOOL isTextStorageType(NSString *type)
{
    return [type isEqualToString:OSAStorageTextType];
}

@implementation OSAScript

@synthesize date = _date;

+ (instancetype)scriptWithSource:(NSString *)source language:(OSALanguage *)language
{
    return [[[self alloc] initWithSource:source language:language] autorelease];
}

+ (instancetype)scriptWithSource:(NSString *)source fromURL:(NSURL *)url languageInstance:(OSALanguageInstance *)instance usingStorageOptions:(OSAStorageOptions)storageOptions
{
    return [[[self alloc] initWithSource:source fromURL:url languageInstance:instance usingStorageOptions:storageOptions] autorelease];
}

+ (instancetype)scriptWithDataDescriptor:(NSAppleEventDescriptor *)data fromURL:(NSURL *)url languageInstance:(OSALanguageInstance *)instance usingStorageOptions:(OSAStorageOptions)storageOptions error:(NSDictionary **)errorInfo
{
    return [[[self alloc] initWithScriptDataDescriptor:data fromURL:url languageInstance:instance usingStorageOptions:storageOptions error:errorInfo] autorelease];
}

- (instancetype)init
{
    return [self initWithSource:@""];
}

- (instancetype)initWithSource:(NSString *)source
{
    return [self initWithSource:source fromURL:nil languageInstance:nil usingStorageOptions:OSANull];
}

- (instancetype)initWithSource:(NSString *)source language:(OSALanguage *)language
{
    OSALanguageInstance *instance = language != nil ? [OSALanguageInstance languageInstanceWithLanguage:language] : nil;
    return [self initWithSource:source fromURL:nil languageInstance:instance usingStorageOptions:OSANull];
}

- (instancetype)initWithSource:(NSString *)source fromURL:(NSURL *)url languageInstance:(OSALanguageInstance *)instance usingStorageOptions:(OSAStorageOptions)storageOptions
{
    self = [super init];
    if (self == nil)
        return nil;
    _source = [(source ?: @"") copy];
    _url = [url copy];
    _languageInstance = [(instance ?: [OSALanguageInstance languageInstanceWithLanguage:[OSALanguage defaultLanguage]]) retain];
    return self;
}

- (instancetype)initWithContentsOfURL:(NSURL *)url error:(NSDictionary **)errorInfo
{
    return [self initWithContentsOfURL:url languageInstance:nil usingStorageOptions:OSANull error:errorInfo];
}

// Only plain-text scripts can be opened; compiled scripts are not UTF-8 and need the engine.
- (instancetype)initWithContentsOfURL:(NSURL *)url languageInstance:(OSALanguageInstance *)instance usingStorageOptions:(OSAStorageOptions)storageOptions error:(NSDictionary **)errorInfo
{
    if (url == nil) {
        setErrorInfo(errorInfo, @"No script location was given.", errOSASystemError);
        [self release];
        return nil;
    }
    NSError *readError = nil;
    NSData *data = [NSData dataWithContentsOfURL:url options:0 error:&readError];
    if (data == nil) {
        setErrorInfo(errorInfo, [readError localizedDescription] ?: @"The script could not be read.", errOSASystemError);
        [self release];
        return nil;
    }
    NSString *source = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
    if (source == nil) {
        setEngineUnavailable(errorInfo);
        [self release];
        return nil;
    }
    return [self initWithSource:source fromURL:url languageInstance:instance usingStorageOptions:storageOptions];
}

- (instancetype)initWithScriptDataDescriptor:(NSAppleEventDescriptor *)data fromURL:(NSURL *)url languageInstance:(OSALanguageInstance *)instance usingStorageOptions:(OSAStorageOptions)storageOptions error:(NSDictionary **)errorInfo
{
    NSString *source = [data stringValue];
    if (source == nil) {
        setEngineUnavailable(errorInfo);
        [self release];
        return nil;
    }
    return [self initWithSource:source fromURL:url languageInstance:instance usingStorageOptions:storageOptions];
}

- (void)dealloc
{
    [_source release];
    [_url release];
    [_languageInstance release];
    [_date release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    return [[[self class] allocWithZone:zone] initWithSource:_source fromURL:_url languageInstance:_languageInstance usingStorageOptions:OSANull];
}

- (NSString *)source
{
    return [[_source retain] autorelease];
}

- (NSString *)sourceAndReturnError:(NSDictionary **)errorInfo
{
    return [self source];
}

- (NSURL *)url
{
    return [[_url retain] autorelease];
}

- (OSALanguage *)language
{
    return [_languageInstance language];
}

- (void)setLanguage:(OSALanguage *)language
{
    [self setLanguageInstance:[OSALanguageInstance languageInstanceWithLanguage:language]];
}

- (OSALanguageInstance *)languageInstance
{
    return [[_languageInstance retain] autorelease];
}

// OSALanguageInstance does not conform to NSCopying, so the declared copy attribute is a retain.
- (void)setLanguageInstance:(OSALanguageInstance *)instance
{
    if (instance == nil)
        instance = [OSALanguageInstance languageInstanceWithLanguage:[OSALanguage defaultLanguage]];
    [instance retain];
    [_languageInstance release];
    _languageInstance = instance;
}

- (BOOL)isCompiled
{
    return NO;
}

- (BOOL)hasOpenHandler
{
    return NO;
}

- (BOOL)compileAndReturnError:(NSDictionary **)errorInfo
{
    setEngineUnavailable(errorInfo);
    return NO;
}

- (NSAppleEventDescriptor *)executeAndReturnError:(NSDictionary **)errorInfo
{
    setEngineUnavailable(errorInfo);
    return nil;
}

- (NSAppleEventDescriptor *)executeAppleEvent:(NSAppleEventDescriptor *)event error:(NSDictionary **)errorInfo
{
    setEngineUnavailable(errorInfo);
    return nil;
}

- (NSAppleEventDescriptor *)executeAndReturnDisplayValue:(NSAttributedString **)displayValue error:(NSDictionary **)errorInfo
{
    if (displayValue != NULL)
        *displayValue = nil;
    setEngineUnavailable(errorInfo);
    return nil;
}

- (NSAttributedString *)richTextSource
{
    return [[[NSAttributedString alloc] initWithString:_source] autorelease];
}

- (NSAttributedString *)richTextFromDescriptor:(NSAppleEventDescriptor *)descriptor
{
    return [_languageInstance richTextFromDescriptor:descriptor];
}

- (NSAttributedString *)richTextFromDescriptorForLog:(NSAppleEventDescriptor *)descriptor
{
    return [self richTextFromDescriptor:descriptor];
}

- (NSData *)compiledDataForType:(NSString *)type usingStorageOptions:(OSAStorageOptions)storageOptions error:(NSDictionary **)errorInfo
{
    if (isTextStorageType(type))
        return [_source dataUsingEncoding:NSUTF8StringEncoding];
    setEngineUnavailable(errorInfo);
    return nil;
}

- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)type error:(NSDictionary **)errorInfo
{
    return [self writeToURL:url ofType:type usingStorageOptions:OSANull error:errorInfo];
}

- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)type usingStorageOptions:(OSAStorageOptions)storageOptions error:(NSDictionary **)errorInfo
{
    if (url == nil) {
        setErrorInfo(errorInfo, @"No script location was given.", errOSASystemError);
        return NO;
    }
    NSData *data = [self compiledDataForType:type usingStorageOptions:storageOptions error:errorInfo];
    if (data == nil)
        return NO;
    NSError *writeError = nil;
    if (![data writeToURL:url options:NSDataWritingAtomic error:&writeError]) {
        setErrorInfo(errorInfo, [writeError localizedDescription] ?: @"The script could not be saved.", errOSASystemError);
        return NO;
    }
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
