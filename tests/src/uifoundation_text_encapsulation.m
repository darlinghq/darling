// Copyright (C) 2026 Lubos Dolezel
//
// NSTextEncapsulation (a value object with copying and secure coding, as the
// system class has: it adopts NSCopying and NSSecureCoding, has the -initWithShape:
// initialiser and does not override isEqual:, all observed on macOS 26.7.3),
// the NSTextEncapsulationAttributeName attribute key with the system's string, and
// NSTextGraphicsContextProvider's class registry and scoped current context.
#import <Foundation/Foundation.h>
#import <UIFoundation/UIFoundation.h>
#include <stdio.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

@interface FakeProvider : NSObject
@end
@implementation FakeProvider
+ (id)graphicsContextForApplicationFrameworkContext:(id)context { return nil; }
+ (Class)colorClassForApplicationFrameworkContext:(id)context { return [NSDate class]; }
@end

@interface PlainProvider : NSObject
@end
@implementation PlainProvider
+ (id)graphicsContextForApplicationFrameworkContext:(id)context { return nil; }
@end

int main(void)
{
	@autoreleasepool {
		CHECK([NSTextEncapsulationAttributeName isEqualToString:@"NSTextEncapsulation"]);

		NSTextEncapsulation *e = [[NSTextEncapsulation alloc] init];
		CHECK(e.scale == 0 && e.shape == 0 && e.style == 0 && e.platterSize == 0);
		CHECK(e.lineWeight == 0 && e.minimumWidth == 0 && e.color == nil);
		e.scale = 2; e.shape = 1; e.style = 1; e.platterSize = 1;
		e.lineWeight = 1.5; e.minimumWidth = 20.25;
		NSMutableString *tint = [NSMutableString stringWithString:@"tint"];
		e.color = tint;
		[tint appendString:@"ed"];
		CHECK([e.color isEqual:@"tint"]);   // the colour is copied

		NSTextEncapsulation *shaped = [[NSTextEncapsulation alloc] initWithShape:2];
		CHECK(shaped.shape == 2 && shaped.scale == 0);

		// NSCopying: an independent copy with the same values; no value equality.
		NSTextEncapsulation *c = [e copy];
		CHECK(c != e && [c class] == [NSTextEncapsulation class]);
		CHECK(c.scale == 2 && c.shape == 1 && c.style == 1 && c.platterSize == 1);
		CHECK(c.lineWeight == 1.5 && c.minimumWidth == 20.25 && [c.color isEqual:@"tint"]);
		c.scale = 0;
		CHECK(e.scale == 2);
		CHECK(![e isEqual:c]);
		CHECK([e isEqual:e]);

		// NSSecureCoding round trip (without a colour: the colour class belongs to AppKit).
		CHECK([NSTextEncapsulation supportsSecureCoding]);
		e.color = nil;
		NSError *error = nil;
		NSData *data = [NSKeyedArchiver archivedDataWithRootObject:e requiringSecureCoding:YES error:&error];
		CHECK(data != nil && error == nil);
		NSTextEncapsulation *back = [NSKeyedUnarchiver unarchivedObjectOfClass:[NSTextEncapsulation class] fromData:data error:&error];
		CHECK(back != nil && error == nil);
		CHECK(back.scale == 2 && back.shape == 1 && back.style == 1 && back.platterSize == 1);
		CHECK(back.lineWeight == 1.5 && back.minimumWidth == 20.25 && back.color == nil);

		// The attribute carries the object in an attributed string.
		NSAttributedString *as = [[NSAttributedString alloc] initWithString:@"abc"
			attributes:@{ NSTextEncapsulationAttributeName: e }];
		CHECK([as attribute:NSTextEncapsulationAttributeName atIndex:1 effectiveRange:NULL] == e);

		// Provider registry.
		CHECK([NSTextGraphicsContextProvider textGraphicsContextProviderClass] == Nil);
		CHECK(![NSTextGraphicsContextProvider textGraphicsContextProviderClassRespondsToColorQuery]);
		[NSTextGraphicsContextProvider setTextGraphicsContextProviderClass:[PlainProvider class]];
		CHECK([NSTextGraphicsContextProvider textGraphicsContextProviderClass] == [PlainProvider class]);
		CHECK(![NSTextGraphicsContextProvider textGraphicsContextProviderClassRespondsToColorQuery]);
		[NSTextGraphicsContextProvider setTextGraphicsContextProviderClass:[FakeProvider class]];
		CHECK([NSTextGraphicsContextProvider textGraphicsContextProviderClassRespondsToColorQuery]);
		CHECK([NSTextGraphicsContextProvider __defaultColorClass] == [NSDate class]);
		[NSTextGraphicsContextProvider setTextGraphicsContextProviderClass:Nil];
		CHECK([NSTextGraphicsContextProvider textGraphicsContextProviderClass] == Nil);

		CHECK([NSTextGraphicsContextProvider textGraphicsContextClass] == Nil);
		[NSTextGraphicsContextProvider setTextGraphicsContextClass:[FakeProvider class]];
		CHECK([NSTextGraphicsContextProvider textGraphicsContextClass] == [FakeProvider class]);
		[NSTextGraphicsContextProvider setTextGraphicsContextClass:Nil];

		// The current-context scope: the block runs, nesting restores, and an exception still unwinds it.
		__block int ran = 0;
		[NSTextGraphicsContextProvider setCurrentTextGraphicsContext:@"outer" duringBlock:^{
			ran++;
			[NSTextGraphicsContextProvider setCurrentTextGraphicsContext:@"inner" duringBlock:^{ ran++; }];
			ran++;
		}];
		CHECK(ran == 3);
		@try {
			[NSTextGraphicsContextProvider setCurrentTextGraphicsContext:@"x" duringBlock:^{ [NSException raise:@"T" format:@"t"]; }];
			CHECK(0);
		} @catch (NSException *ex) {
			CHECK([[ex name] isEqual:@"T"]);
		}
		[NSTextGraphicsContextProvider setCurrentTextGraphicsContext:@"y" duringBlock:nil];
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
