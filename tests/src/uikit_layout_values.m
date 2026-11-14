/*
 * Copyright (C) 2026 Darling Developers
 *
 * Contract: the NSCollectionLayoutDimension, NSCollectionLayoutSize,
 * NSCollectionLayoutSpacing and NSCollectionLayoutEdgeSpacing value classes
 * from UIKit's public documentation.  The classes are looked up at run time so
 * the failure without UIKit installed is a missing class, not a link error.
 */
#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#include <dlfcn.h>
#include <math.h>
#include <stdio.h>

#define CHECK(condition) do { if (!(condition)) { printf("FAIL line %d: %s\n", __LINE__, #condition); return 1; } } while (0)

static BOOL Raises(void (^block)(void))
{
    @try { block(); }
    @catch (NSException *e) { return [e.name isEqual:NSInvalidArgumentException]; }
    return NO;
}
#define RAISES(expr) Raises(^{ (void)(expr); })

int main(void)
{
    @autoreleasepool {
        /* UIKit is an iOSSupport framework, so it is not linked by default. */
        if (!dlopen("/System/iOSSupport/System/Library/Frameworks/UIKit.framework/UIKit", RTLD_NOW | RTLD_GLOBAL)) {
            printf("FAIL: dlopen: %s\n", dlerror());
            return 1;
        }
        Class dimension = NSClassFromString(@"NSCollectionLayoutDimension");
        Class size = NSClassFromString(@"NSCollectionLayoutSize");
        Class spacing = NSClassFromString(@"NSCollectionLayoutSpacing");
        Class edges = NSClassFromString(@"NSCollectionLayoutEdgeSpacing");
        CHECK(dimension && size && spacing && edges);
        NSCollectionLayoutDimension *width = [dimension fractionalWidthDimension:0.25];
        NSCollectionLayoutDimension *height = [dimension absoluteDimension:44];
        CHECK(width.dimension == 0.25 && width.isFractionalWidth);
        CHECK(!width.isFractionalHeight && !width.isAbsolute && !width.isEstimated);
        CHECK(height.dimension == 44 && height.isAbsolute);
        NSCollectionLayoutDimension *estimated = [dimension estimatedDimension:120];
        CHECK(estimated.isEstimated && estimated.dimension == 120);
        NSCollectionLayoutDimension *fractionalHeight = [dimension fractionalHeightDimension:0.5];
        CHECK(fractionalHeight.isFractionalHeight && fractionalHeight.dimension == 0.5);
        NSCollectionLayoutSize *layoutSize = [size sizeWithWidthDimension:width heightDimension:height];
        CHECK(layoutSize.widthDimension.dimension == 0.25 && layoutSize.heightDimension.dimension == 44);
        NSCollectionLayoutSpacing *fixed = [spacing fixedSpacing:8];
        NSCollectionLayoutSpacing *flexible = [spacing flexibleSpacing:3];
        CHECK(fixed.isFixedSpacing && !fixed.isFlexibleSpacing && fixed.spacing == 8);
        CHECK(flexible.isFlexibleSpacing && !flexible.isFixedSpacing && flexible.spacing == 3);
        NSCollectionLayoutEdgeSpacing *edge = [edges spacingForLeading:fixed top:nil trailing:flexible bottom:nil];
        CHECK(edge.leading.spacing == 8 && edge.top == nil && edge.trailing.spacing == 3 && edge.bottom == nil);
        NSCollectionLayoutEdgeSpacing *copy = [edge copy];
        CHECK(copy.leading.spacing == 8 && copy.trailing.isFlexibleSpacing);
        NSCollectionLayoutSize *sizeCopy = [layoutSize copy];
        CHECK(sizeCopy.heightDimension.dimension == 44);
        CHECK(RAISES([size sizeWithWidthDimension:nil heightDimension:height]));
        CHECK(RAISES([size sizeWithWidthDimension:width heightDimension:nil]));
        CHECK(RAISES([size sizeWithWidthDimension:(id)@"x" heightDimension:height]));

        // Rung-6 policy: pin each rejection and each deliberately allowed boundary.
        CHECK(RAISES([dimension absoluteDimension:NAN]));
        CHECK(RAISES([dimension estimatedDimension:INFINITY]));
        CHECK(RAISES([dimension fractionalWidthDimension:-INFINITY]));
        CHECK(RAISES([dimension fractionalHeightDimension:NAN]));
        CHECK(RAISES([dimension absoluteDimension:-1]));
        CHECK(RAISES([dimension fractionalWidthDimension:-0.5]));
        CHECK(((NSCollectionLayoutDimension *)[dimension absoluteDimension:0]).dimension == 0);
        CHECK(RAISES([spacing fixedSpacing:NAN]));
        CHECK(RAISES([spacing flexibleSpacing:INFINITY]));
        CHECK(((NSCollectionLayoutSpacing *)[spacing fixedSpacing:-4]).spacing == -4);
        CHECK(RAISES([edges spacingForLeading:(id)@"x" top:nil trailing:nil bottom:nil]));
        CHECK(RAISES([edges spacingForLeading:nil top:nil trailing:nil bottom:(id)width]));
        for (Class cls in @[dimension, size, spacing, edges]) {
            CHECK(RAISES([cls performSelector:NSSelectorFromString(@"new")]));
            CHECK(RAISES([[cls alloc] performSelector:NSSelectorFromString(@"init")]));
        }
        puts("PASS: UIKit layout value factories, types, ownership and copies");
    }
    return 0;
}
