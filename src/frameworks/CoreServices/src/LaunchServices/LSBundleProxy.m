/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// Proxies describing bundles, as LaunchServices hands them out (macOS SPI). The hierarchy
// (_LSQueryResult, LSResourceProxy, LSBundleProxy) and the errors of
// +bundleProxyWithAuditToken:error: follow observations on macOS 26.7.3.
// Test: tests/launchservices-bundle-proxy.m.

#import "LSBundleProxy.h"
#import <Foundation/Foundation.h>
#include <bsm/libbsm.h>
#include <libproc.h>
#include <limits.h>
#include <CarbonCore/MacErrors.h>

@implementation _LSQueryResult

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [self retain];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    return [self init];
}

@end

@implementation LSResourceProxy

- (NSString *)localizedName
{
    return nil;
}

@end

@implementation LSBundleProxy

- (instancetype)_initWithBundle:(NSBundle *)bundle
{
    self = [super init];
    if (self != nil)
        _bundleURL = [[bundle bundleURL] copy];
    return self;
}

- (void)dealloc
{
    [_bundleURL release];
    [super dealloc];
}

- (NSBundle *)_bundle
{
    return _bundleURL != nil ? [NSBundle bundleWithURL:_bundleURL] : nil;
}

- (NSURL *)bundleURL
{
    return _bundleURL;
}

- (NSString *)bundleIdentifier
{
    return [[self _bundle] bundleIdentifier];
}

// The bundle's file name without its extension; CFBundleName and CFBundleDisplayName are not
// used (observed on macOS 26.7.3).
- (NSString *)localizedName
{
    return [[_bundleURL lastPathComponent] stringByDeletingPathExtension];
}

- (BOOL)isEqual:(id)other
{
    return [other isKindOfClass:[LSBundleProxy class]] && [((LSBundleProxy *) other)->_bundleURL isEqual:_bundleURL];
}

- (NSUInteger)hash
{
    return [_bundleURL hash];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_bundleURL forKey:@"bundleURL"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self != nil)
        _bundleURL = [[coder decodeObjectOfClass:[NSURL class] forKey:@"bundleURL"] copy];
    return self;
}

static NSError *proxyError(OSStatus status, NSString *description)
{
    NSDictionary *userInfo = description != nil ? @{ NSDebugDescriptionErrorKey: description } : nil;
    return [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:userInfo];
}

// The bundle whose executable the file at the path is: the executable sits in Contents/MacOS of
// the bundle, or directly in it for a flat bundle.
static NSBundle *bundleForExecutable(NSString *path)
{
    NSString *executable = [path stringByResolvingSymlinksInPath];
    NSString *directory = [executable stringByDeletingLastPathComponent];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSMutableArray *candidates = [NSMutableArray array];
    // A bundle has an information property list: Contents/Info.plist, or Info.plist at the top of
    // a flat bundle. A directory holding a tool without one is not a bundle.
    if ([[directory lastPathComponent] isEqualToString:@"MacOS"]
        && [[[directory stringByDeletingLastPathComponent] lastPathComponent] isEqualToString:@"Contents"]
        && [fileManager fileExistsAtPath:[[directory stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"Info.plist"]])
        [candidates addObject:[[directory stringByDeletingLastPathComponent] stringByDeletingLastPathComponent]];
    if ([fileManager fileExistsAtPath:[directory stringByAppendingPathComponent:@"Info.plist"]])
        [candidates addObject:directory];

    for (NSString *candidate in candidates) {
        NSBundle *bundle = [NSBundle bundleWithPath:candidate];
        NSString *bundleExecutable = [[bundle executablePath] stringByResolvingSymlinksInPath];
        if (bundleExecutable != nil && [bundleExecutable isEqualToString:executable])
            return bundle;
    }
    return nil;
}

// The bundle of the process the audit token names. Fails with procNotFound when there is no such
// process, and with paramErr when its executable is not a bundle's.
+ (instancetype)bundleProxyWithAuditToken:(audit_token_t)auditToken error:(NSError **)outError
{
    pid_t pid = audit_token_to_pid(auditToken);
    char path[PROC_PIDPATHINFO_MAXSIZE];
    if (pid <= 0 || proc_pidpath(pid, path, sizeof path) <= 0) {
        if (outError != NULL)
            *outError = proxyError(procNotFound, nil);
        return nil;
    }

    NSBundle *bundle = bundleForExecutable([[NSFileManager defaultManager] stringWithFileSystemRepresentation:path length:strlen(path)]);
    if (bundle == nil) {
        if (outError != NULL)
            *outError = proxyError(paramErr, @"This process is not the executable of a bundle.");
        return nil;
    }
    return [[[self alloc] _initWithBundle:bundle] autorelease];
}

+ (instancetype)bundleProxyForCurrentProcess
{
    NSBundle *bundle = bundleForExecutable([[NSBundle mainBundle] executablePath]);
    return bundle != nil ? [[[self alloc] _initWithBundle:bundle] autorelease] : nil;
}

@end
