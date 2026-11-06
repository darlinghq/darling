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

#import <StoreKit/SKPaymentQueue.h>

@implementation SKPaymentQueue {
    NSMutableArray *_observers;
}

// There is no App Store to talk to: payments can never be made and the queue
// never contains transactions. Observers are retained but never notified.
+ (SKPaymentQueue *)defaultQueue
{
    static SKPaymentQueue *queue;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        queue = [[SKPaymentQueue alloc] init];
    });
    return queue;
}

+ (BOOL)canMakePayments
{
    return NO;
}

- (instancetype)init
{
    if ((self = [super init])) {
        _observers = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)addTransactionObserver:(id<SKPaymentTransactionObserver>)observer
{
    if (observer != nil && ![_observers containsObject:observer]) {
        [_observers addObject:observer];
    }
}

- (void)removeTransactionObserver:(id<SKPaymentTransactionObserver>)observer
{
    [_observers removeObject:observer];
}

- (NSArray *)transactions
{
    return @[];
}

- (void)restoreCompletedTransactions
{
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
