/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

#import <Foundation/Foundation.h>

@class WKHTTPCookieStore, WKWebsiteDataRecord;

NS_ASSUME_NONNULL_BEGIN

extern NSString * const WKWebsiteDataTypeFetchCache;
extern NSString * const WKWebsiteDataTypeDiskCache;
extern NSString * const WKWebsiteDataTypeMemoryCache;
extern NSString * const WKWebsiteDataTypeOfflineWebApplicationCache;
extern NSString * const WKWebsiteDataTypeCookies;
extern NSString * const WKWebsiteDataTypeSessionStorage;
extern NSString * const WKWebsiteDataTypeLocalStorage;
extern NSString * const WKWebsiteDataTypeWebSQLDatabases;
extern NSString * const WKWebsiteDataTypeIndexedDBDatabases;
extern NSString * const WKWebsiteDataTypeServiceWorkerRegistrations;
extern NSString * const WKWebsiteDataTypeFileSystem;

@interface WKWebsiteDataStore : NSObject

@property (class, nonatomic, readonly) WKWebsiteDataStore *defaultDataStore;
+ (WKWebsiteDataStore *)nonPersistentDataStore;

@property (nonatomic, readonly, getter=isPersistent) BOOL persistent;
@property (nonatomic, readonly) WKHTTPCookieStore *httpCookieStore;

+ (NSSet<NSString *> *)allWebsiteDataTypes;

- (void)fetchDataRecordsOfTypes:(NSSet<NSString *> *)dataTypes completionHandler:(void (^)(NSArray<WKWebsiteDataRecord *> *))completionHandler;
- (void)removeDataOfTypes:(NSSet<NSString *> *)dataTypes forDataRecords:(NSArray<WKWebsiteDataRecord *> *)dataRecords completionHandler:(void (^)(void))completionHandler;
- (void)removeDataOfTypes:(NSSet<NSString *> *)websiteDataTypes modifiedSince:(NSDate *)date completionHandler:(void (^)(void))completionHandler;

@end

NS_ASSUME_NONNULL_END
