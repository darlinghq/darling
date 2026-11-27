// Copyright (C) 2026 Lubos Dolezel
//
// Payments are not available, and SKPaymentQueue reports that the documented
// way: +canMakePayments is NO, the default queue is a shared object that holds
// no transactions, and observers can be added and removed (including repeatedly
// and when not registered) without being called.
#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>
#include <stdio.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

@interface Observer : NSObject <SKPaymentTransactionObserver> {
@public
	int calls;
}
@end

@implementation Observer
- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions { calls++; }
@end

int main(void)
{
	@autoreleasepool {
		CHECK([SKPaymentQueue canMakePayments] == NO);

		SKPaymentQueue *queue = [SKPaymentQueue defaultQueue];
		CHECK(queue != nil);
		CHECK([queue isKindOfClass:[SKPaymentQueue class]]);
		CHECK(queue == [SKPaymentQueue defaultQueue]);

		NSArray *transactions = queue.transactions;
		CHECK(transactions != nil);
		CHECK([transactions count] == 0);

		Observer *observer = [[Observer alloc] init];
		[queue addTransactionObserver:observer];
		[queue addTransactionObserver:observer];
		CHECK([queue.transactions count] == 0);
		[queue removeTransactionObserver:observer];
		[queue removeTransactionObserver:observer];
		[queue removeTransactionObserver:[[Observer alloc] init]];
		CHECK(observer->calls == 0);
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
