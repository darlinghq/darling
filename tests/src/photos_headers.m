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

// Photos declarations: PHFetchResult is a generic, fast-enumerable, copyable collection class and
// PHProjectChangeRequest derives from PHChangeRequest. The class implementations are forwarding
// stubs, so only the class graph and the declared protocol conformance are checked at run time;
// the generic spelling is checked by compiling this file (a lightweight generic use below).
// Prints PASS when everything holds; the exit code is the number of failed checks.

#import <Foundation/Foundation.h>
#import <Photos/Photos.h>
#import <objc/runtime.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

// Compile-time check of the generic spelling and the block signatures.
static NSUInteger sum_indexes(PHFetchResult<PHAsset *> *result)
{
    __block NSUInteger total = 0;
    [result enumerateObjectsUsingBlock:^(PHAsset *asset, NSUInteger idx, BOOL *stop) {
        (void)asset;
        total += idx;
    }];
    PHAsset *first = result.firstObject;
    (void)first;
    return total + result.count;
}

int main(void)
{
    @autoreleasepool {
        CHECK([PHFetchResult class] != Nil);
        CHECK(class_conformsToProtocol([PHFetchResult class], @protocol(NSCopying)));
        CHECK(class_conformsToProtocol([PHFetchResult class], @protocol(NSFastEnumeration)));
        CHECK(class_getSuperclass([PHProjectChangeRequest class]) == [PHChangeRequest class]);
        CHECK(sum_indexes != NULL);
    }
    if (!failures) printf("PASS\n");
    return failures;
}
