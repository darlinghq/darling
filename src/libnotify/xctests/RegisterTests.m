//
//  xctests.m
//  xctests
//

#include <dispatch/dispatch.h>
#include <notify.h>
#include <stdatomic.h>
#include <unistd.h>

#include "notify_internal.h"

#import <XCTest/XCTest.h>

@interface RegisterTests : XCTestCase

@end

@implementation RegisterTests

static dispatch_queue_t noteQueue;

+ (void)setUp
{
	noteQueue = dispatch_queue_create("noteQ", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
}

+ (void)tearDown
{
	noteQueue = nil;
}

- (void)tearDown
{
	[super tearDown];
}

- (void)test00RegisterSimple
{	
	static int token;
	XCTAssert(notify_register_dispatch("com.example.test.simple", &token, noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
	XCTAssert(notify_cancel(token) == NOTIFY_STATUS_OK, @"notify_cancel failed");
}

- (void)test01RegisterNested
{	
	for (int i = 0; i < 100000; i++) {
		static int token1, token2;
		XCTAssert(notify_register_dispatch("com.example.test.multiple", &token1, noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
		XCTAssert(notify_register_dispatch("com.example.test.multiple", &token2, noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
		XCTAssert(notify_cancel(token1) == NOTIFY_STATUS_OK, @"notify_cancel failed");
		XCTAssert(notify_cancel(token2) == NOTIFY_STATUS_OK, @"notify_cancel failed");
	}
}

- (void)test02RegisterInterleaved
{
	for (int i = 0; i < 100000; i++) {
		static int token1, token2;
		XCTAssert(notify_register_dispatch("com.example.test.interleaved", &token1, noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
		XCTAssert(notify_cancel(token1) == NOTIFY_STATUS_OK, @"notify_cancel failed");
		XCTAssert(notify_register_dispatch("com.example.test.interleaved", &token2, noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
		XCTAssert(notify_cancel(token2) == NOTIFY_STATUS_OK, @"notify_cancel failed");
	}
}

- (void)test03RegisterRaceWithDealloc
{	
	static int tokens[1000000];
	dispatch_apply(1000000, DISPATCH_APPLY_AUTO, ^(size_t i) {
		XCTAssert(notify_register_check("com.example.test.race", &tokens[i]) == NOTIFY_STATUS_OK, @"notify_register_check failed");
		XCTAssert(notify_cancel(tokens[i]) == NOTIFY_STATUS_OK, @"notify_cancel failed");
		XCTAssert(notify_register_dispatch("com.example.test.race", &tokens[i], noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
		XCTAssert(notify_cancel(tokens[i]) == NOTIFY_STATUS_OK, @"notify_cancel failed");
	    	XCTAssert(notify_post("com.example.test.race") == NOTIFY_STATUS_OK, @"notify_post failed");
	});
}

- (void)test04RegisterManyTokens
{	
	static int tokens[100000];
	dispatch_apply(100000, DISPATCH_APPLY_AUTO, ^(size_t i) {
		XCTAssert(notify_register_dispatch("com.example.test.many", &tokens[i], noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
		XCTAssert(notify_cancel(tokens[i]) == NOTIFY_STATUS_OK, @"notify_cancel failed");
	});
}

- (void)test05RegisterBulkCancel
{	
    static int tokens[100000];
	dispatch_apply(100000, DISPATCH_APPLY_AUTO, ^(size_t i) {
		XCTAssert(notify_register_dispatch("com.example.test.bulk", &tokens[i], noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
	});
	
	dispatch_apply(100000, DISPATCH_APPLY_AUTO, ^(size_t i) {
		XCTAssert(notify_cancel(tokens[i]) == NOTIFY_STATUS_OK, @"notify_cancel failed");
	});
}

- (void)test06RegisterSimpleSelf
{
	static int token;
	XCTAssert(notify_register_dispatch("self.example.test.simple", &token, noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
	XCTAssert(notify_cancel(token) == NOTIFY_STATUS_OK, @"notify_cancel failed");
}

- (void)test07RegisterNestedSelf
{
	for (int i = 0; i < 100000; i++) {
		static int token1, token2;
		XCTAssert(notify_register_dispatch("self.example.test.multiple", &token1, noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
		XCTAssert(notify_register_dispatch("self.example.test.multiple", &token2, noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
		XCTAssert(notify_cancel(token1) == NOTIFY_STATUS_OK, @"notify_cancel failed");
		XCTAssert(notify_cancel(token2) == NOTIFY_STATUS_OK, @"notify_cancel failed");
	}
}

- (void)test08RegisterInterleavedSelf
{
	for (int i = 0; i < 100000; i++) {
		static int token1, token2;
		XCTAssert(notify_register_dispatch("self.example.test.interleaved", &token1, noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
		XCTAssert(notify_cancel(token1) == NOTIFY_STATUS_OK, @"notify_cancel failed");
		XCTAssert(notify_register_dispatch("self.example.test.interleaved", &token2, noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
		XCTAssert(notify_cancel(token2) == NOTIFY_STATUS_OK, @"notify_cancel failed");
	}
}

- (void)test09RegisterRaceWithDeallocSelf
{
	static int tokens[1000000];
	dispatch_apply(1000000, DISPATCH_APPLY_AUTO, ^(size_t i) {
		XCTAssert(notify_register_check("self.example.test.race", &tokens[i]) == NOTIFY_STATUS_OK, @"notify_register_check failed");
		XCTAssert(notify_cancel(tokens[i]) == NOTIFY_STATUS_OK, @"notify_cancel failed");
		XCTAssert(notify_register_dispatch("self.example.test.race", &tokens[i], noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
		XCTAssert(notify_cancel(tokens[i]) == NOTIFY_STATUS_OK, @"notify_cancel failed");
		XCTAssert(notify_post("com.example.test.race") == NOTIFY_STATUS_OK, @"notify_post failed");
	});
}

- (void)test10RegisterManyTokensSelf
{
	static int tokens[100000];
	dispatch_apply(100000, DISPATCH_APPLY_AUTO, ^(size_t i) {
		XCTAssert(notify_register_dispatch("self.example.test.many", &tokens[i], noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
		XCTAssert(notify_cancel(tokens[i]) == NOTIFY_STATUS_OK, @"notify_cancel failed");
	});
}

- (void)test11RegisterBulkCancelSelf
{
	static int tokens[100000];
	dispatch_apply(100000, DISPATCH_APPLY_AUTO, ^(size_t i) {
		XCTAssert(notify_register_dispatch("self.example.test.bulk", &tokens[i], noteQueue, ^(int i){}) == NOTIFY_STATUS_OK, @"notify_register_dispatch failed");
	});

	dispatch_apply(100000, DISPATCH_APPLY_AUTO, ^(size_t i) {
		XCTAssert(notify_cancel(tokens[i]) == NOTIFY_STATUS_OK, @"notify_cancel failed");
	});
}

@end
