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


#ifndef _StoreKit_H_
#define _StoreKit_H_

#import <Foundation/Foundation.h>

#import <StoreKit/SKInAppClient.h>
#import <StoreKit/SKInAppService.h>
#import <StoreKit/SKServiceProxy.h>
#import <StoreKit/SKPaymentQueueClient.h>
#import <StoreKit/SKProductSubscriptionPeriodInternal.h>
#import <StoreKit/SKProductSubscriptionPeriod.h>
#import <StoreKit/SKProduct.h>
#import <StoreKit/SKProductInternal.h>
#import <StoreKit/SKXPCConnection.h>
#import <StoreKit/SKStoreReviewController.h>
#import <StoreKit/SKProductsRequest.h>
#import <StoreKit/SKProductsResponse.h>
#import <StoreKit/SKProductsRequestInternal.h>
#import <StoreKit/SKProductsResponseInternal.h>
#import <StoreKit/SKPayment.h>
#import <StoreKit/SKMutablePayment.h>
#import <StoreKit/SKPaymentInternal.h>
#import <StoreKit/SKDownloadInternal.h>
#import <StoreKit/SKDownload.h>
#import <StoreKit/SKDownloadChangeset.h>
#import <StoreKit/SKPaymentTransaction.h>
#import <StoreKit/SKPaymentTransactionInternal.h>
#import <StoreKit/SKPaymentQueue.h>
#import <StoreKit/SKPaymentQueueInternal.h>
#import <StoreKit/SKRequest.h>
#import <StoreKit/SKRequestInternal.h>
#import <StoreKit/SKProductDiscountInternal.h>
#import <StoreKit/SKProductDiscount.h>
#import <StoreKit/SKReceiptRefreshRequest.h>
#import <StoreKit/SKHandleInvalidReceiptRequest.h>
#import <StoreKit/SKServiceClient.h>
#import <StoreKit/SKPaymentDiscountInternal.h>
#import <StoreKit/SKPaymentDiscount.h>

void* SKTerminateForInvalidReceipt(void);

#endif
