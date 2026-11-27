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

#include <Foundation/Foundation.h>

@class SKPaymentQueue;
@class SKPaymentTransaction;

@protocol SKPaymentTransactionObserver <NSObject>
- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray<SKPaymentTransaction *> *)transactions;
@optional
- (void)paymentQueue:(SKPaymentQueue *)queue removedTransactions:(NSArray<SKPaymentTransaction *> *)transactions;
- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error;
- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue;
@end

@interface SKPaymentQueue : NSObject

// Payments are not available: canMakePayments is NO and the queue never holds
// transactions or notifies observers.
+ (SKPaymentQueue *)defaultQueue;
+ (BOOL)canMakePayments;
@property (nonatomic, readonly) NSArray<SKPaymentTransaction *> *transactions;
- (void)addTransactionObserver:(id<SKPaymentTransactionObserver>)observer;
- (void)removeTransactionObserver:(id<SKPaymentTransactionObserver>)observer;

@end
