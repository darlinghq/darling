#import <UIKit/UIKit.h>
#import <Foundation/NSException.h>
#include <dlfcn.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#import <objc/runtime.h>

#define CHECK(condition) do { if (!(condition)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); return 1; } } while (0)

#define RAISES(expr) ({ BOOL raised_ = NO; \
    @try { (void)(expr); } \
    @catch (NSException *e_) { raised_ = [e_.name isEqual:NSInvalidArgumentException]; } \
    raised_; })

int main(int argc, char **argv)
{
    @autoreleasepool {
        if (argc == 2 && !dlopen(argv[1], RTLD_NOW | RTLD_GLOBAL)) {
            fprintf(stderr, "dlopen: %s\n", dlerror());
            return 2;
        }
        Class dimension = NSClassFromString(@"NSCollectionLayoutDimension");
        Class size = NSClassFromString(@"NSCollectionLayoutSize");
        Class spacing = NSClassFromString(@"NSCollectionLayoutSpacing");
        Class edges = NSClassFromString(@"NSCollectionLayoutEdgeSpacing");
        CHECK(dimension && size && spacing && edges);
        if (argc == 2) {
            // The classes must come from the framework binary under test, not a stand-in.
            for (Class cls in @[dimension, size, spacing, edges]) {
                const char *image = class_getImageName(cls);
                CHECK(image && strstr(image, "UIKit.framework/Versions/A/UIKit"));
            }
        }
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
        CHECK([dimension absoluteDimension:0].dimension == 0);
        CHECK(RAISES([spacing fixedSpacing:NAN]));
        CHECK(RAISES([spacing flexibleSpacing:INFINITY]));
        CHECK([spacing fixedSpacing:-4].spacing == -4);
        CHECK(RAISES([edges spacingForLeading:(id)@"x" top:nil trailing:nil bottom:nil]));
        CHECK(RAISES([edges spacingForLeading:nil top:nil trailing:nil bottom:(id)width]));
        for (Class cls in @[dimension, size, spacing, edges]) {
            CHECK(RAISES([cls performSelector:NSSelectorFromString(@"new")]));
            CHECK(RAISES([[cls alloc] performSelector:NSSelectorFromString(@"init")]));
        }
        puts("PASS UIKit layout value factories, types, ownership and copies");
    }
    return 0;
}
