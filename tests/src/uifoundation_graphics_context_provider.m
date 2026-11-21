// CFLAGS: -framework Foundation
// Copyright (C) 2026 Lubos Dolezel
//
// The private graphics context provider in UIFoundation: with no provider
// registered the current context is NULL and both flags are NO; a registered
// provider is retained and answers for the current context; registering
// another one releases the first; registering nil unregisters.
//
// Runs without AppKit (UIFoundation is opened by path) and therefore checks the
// unregistered state, which is what any process sees that uses UIFoundation
// without AppKit. Prints PASS or FAIL lines, then ALL PASSED or FAILED n.
#import <Foundation/Foundation.h>
#include <dlfcn.h>
#include <stdio.h>

#define UIFOUNDATION_PATH "/System/Library/PrivateFrameworks/UIFoundation.framework/UIFoundation"

static int failures;
static void expect(BOOL ok, const char *what)
{
    printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok)
        failures++;
}

@protocol _UIFoundationGraphicsContextProvider <NSObject>
- (void *)currentCGContext;
- (BOOL)currentContextIsFlipped;
- (BOOL)currentContextIsDrawingToScreen;
@end

@interface TestProvider : NSObject <_UIFoundationGraphicsContextProvider> {
@public
    void *context;
    BOOL flipped, screen;
}
@end

static int deallocCount;

@implementation TestProvider
- (void *)currentCGContext { return context; }
- (BOOL)currentContextIsFlipped { return flipped; }
- (BOOL)currentContextIsDrawingToScreen { return screen; }
- (void)dealloc
{
    deallocCount++;
    [super dealloc];
}
@end

int main(void)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    void *uif = dlopen(UIFOUNDATION_PATH, RTLD_LAZY);
    expect(uif != NULL, "UIFoundation opens");
    if (uif == NULL)
        return 1;

    void (*setProvider)(id) = dlsym(uif, "_UIFoundationSetGraphicsContextProvider");
    id (*getProvider)(void) = dlsym(uif, "_UIFoundationGraphicsContextProvider");
    void *(*currentContext)(void) = dlsym(uif, "_UIFoundationCurrentCGContext");
    BOOL (*isFlipped)(void) = dlsym(uif, "_UIFoundationCurrentContextIsFlipped");
    BOOL (*isScreen)(void) = dlsym(uif, "_UIFoundationCurrentContextIsDrawingToScreen");
    expect(setProvider && getProvider && currentContext && isFlipped && isScreen, "provider functions exported");
    if (!(setProvider && getProvider && currentContext && isFlipped && isScreen))
        return 1;

    expect(getProvider() == nil, "no provider registered without AppKit");
    expect(currentContext() == NULL, "no current context without a provider");
    expect(!isFlipped() && !isScreen(), "flags are NO without a provider");

    TestProvider *first = [[TestProvider alloc] init];
    first->context = (void *) 0x1000;
    first->flipped = YES;
    first->screen = YES;
    setProvider(first);
    expect(getProvider() == first, "registered provider is returned");
    expect([first retainCount] == 2, "provider is retained");
    expect(currentContext() == (void *) 0x1000, "context comes from the provider");
    expect(isFlipped() && isScreen(), "flags come from the provider");

    first->context = (void *) 0x2000;
    first->flipped = NO;
    expect(currentContext() == (void *) 0x2000 && !isFlipped(), "provider is asked on every call");

    TestProvider *second = [[TestProvider alloc] init];
    [first release];
    setProvider(second);
    expect(deallocCount == 1, "replacing the provider releases the previous one");
    expect(getProvider() == second && currentContext() == NULL && !isScreen(), "replacement provider answers");

    [second release];
    setProvider(nil);
    expect(deallocCount == 2, "unregistering releases the provider");
    expect(getProvider() == nil && currentContext() == NULL, "unregistered again");

    [pool release];
    if (failures)
        printf("FAILED %d\n", failures);
    else
        printf("ALL PASSED\n");
    return failures != 0;
}
