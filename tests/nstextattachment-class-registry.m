// Copyright (C) 2026 Lubos Dolezel
//
// +[NSTextAttachment registerTextAttachmentClass:forFileType:] and +textAttachmentClassForFileType:.
// Observed on macOS 26.7.3 (black-box, tier 2; this file was run there): a class registered for a file type is
// returned by the lookup for that type, a later registration for the same type replaces it, lookups are exact
// (case sensitive) and an unregistered type gives Nil. No check is made that the registered class is a subclass of
// NSTextAttachment: any class is stored and returned. A Nil class and a nil file type are ignored without raising.
// No display needed. MRC.
#import <AppKit/AppKit.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

@interface RegistryAttachmentA : NSTextAttachment
@end
@implementation RegistryAttachmentA
@end

@interface RegistryAttachmentB : NSTextAttachment
@end
@implementation RegistryAttachmentB
@end

@interface RegistryNotAnAttachment : NSObject
@end
@implementation RegistryNotAnAttachment
@end

int main(void)
{
    @autoreleasepool
    {
        Class base = [NSTextAttachment class];

        CHECK([base textAttachmentClassForFileType:@"registry.unregistered"] == Nil);

        [base registerTextAttachmentClass:[RegistryAttachmentA class] forFileType:@"registry.a"];
        CHECK([base textAttachmentClassForFileType:@"registry.a"] == [RegistryAttachmentA class]);
        CHECK([base textAttachmentClassForFileType:@"REGISTRY.A"] == Nil);
        CHECK([base textAttachmentClassForFileType:@"registry.b"] == Nil);

        [base registerTextAttachmentClass:[RegistryAttachmentB class] forFileType:@"registry.a"];
        CHECK([base textAttachmentClassForFileType:@"registry.a"] == [RegistryAttachmentB class]);

        // a subclass registers and looks up through itself too: the registry is the base class's
        CHECK([RegistryAttachmentA textAttachmentClassForFileType:@"registry.a"] == [RegistryAttachmentB class]);

        // not checked to be a text attachment class
        BOOL raised = NO;
        @try
        {
            [base registerTextAttachmentClass:[RegistryNotAnAttachment class] forFileType:@"registry.other"];
        }
        @catch (NSException *e)
        {
            raised = YES;
        }
        CHECK(!raised);
        CHECK([base textAttachmentClassForFileType:@"registry.other"] == [RegistryNotAnAttachment class]);

        // Nil class and nil type are ignored: nothing raised, nothing changed
        raised = NO;
        @try
        {
            [base registerTextAttachmentClass:Nil forFileType:@"registry.a"];
            [base registerTextAttachmentClass:Nil forFileType:@"registry.fresh"];
            [base registerTextAttachmentClass:[RegistryAttachmentA class] forFileType:nil];
        }
        @catch (NSException *e)
        {
            raised = YES;
        }
        CHECK(!raised);
        CHECK([base textAttachmentClassForFileType:@"registry.a"] == [RegistryAttachmentB class]);
        CHECK([base textAttachmentClassForFileType:@"registry.fresh"] == Nil);
        CHECK([base textAttachmentClassForFileType:nil] == Nil);
    }
    if (failures == 0)
        printf("PASS nstextattachment-class-registry\n");
    return failures != 0;
}
