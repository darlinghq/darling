project(StoreKit)

remove_sdk_framework(StoreKit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "379.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(StoreKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/StoreKit"
)

add_framework(StoreKit
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/StoreKit.m
        src/SKServiceProxy.m
        src/SKPaymentQueueClient.m
        src/SKProductSubscriptionPeriodInternal.m
        src/SKProductSubscriptionPeriod.m
        src/SKProduct.m
        src/SKProductInternal.m
        src/SKXPCConnection.m
        src/SKStoreReviewController.m
        src/SKProductsRequest.m
        src/SKProductsResponse.m
        src/SKProductsRequestInternal.m
        src/SKProductsResponseInternal.m
        src/SKPayment.m
        src/SKMutablePayment.m
        src/SKPaymentInternal.m
        src/SKDownloadInternal.m
        src/SKDownload.m
        src/SKDownloadChangeset.m
        src/SKPaymentTransaction.m
        src/SKPaymentTransactionInternal.m
        src/SKPaymentQueue.m
        src/SKPaymentQueueInternal.m
        src/SKRequest.m
        src/SKRequestInternal.m
        src/SKProductDiscountInternal.m
        src/SKProductDiscount.m
        src/SKReceiptRefreshRequest.m
        src/SKHandleInvalidReceiptRequest.m
        src/SKServiceClient.m
        src/SKPaymentDiscountInternal.m
        src/SKPaymentDiscount.m

    DEPENDENCIES
        system
        objc
        Foundation
)
