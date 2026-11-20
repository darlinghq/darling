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

#import <Foundation/Foundation.h>

#ifndef NS_REFINED_FOR_SWIFT
#if __has_attribute(swift_private)
#define NS_REFINED_FOR_SWIFT __attribute__((swift_private))
#else
#define NS_REFINED_FOR_SWIFT
#endif
#endif

#ifndef NS_ERROR_ENUM
#define __NS_ERROR_ENUM_GET_MACRO(_1, _2, NAME, ...) NAME
#if ((__cplusplus && __cplusplus >= 201103L && (__has_extension(cxx_strong_enums) || __has_feature(objc_fixed_enum))) || (!__cplusplus && __has_feature(objc_fixed_enum))) && __has_attribute(ns_error_domain)
#define __NS_NAMED_ERROR_ENUM(_domain, _name)     enum _name : NSInteger _name; enum __attribute__((ns_error_domain(_domain))) _name : NSInteger
#define __NS_ANON_ERROR_ENUM(_domain)             enum __attribute__((ns_error_domain(_domain))) : NSInteger
#else
#define __NS_NAMED_ERROR_ENUM(_domain, _name) NS_ENUM(NSInteger, _name)
#define __NS_ANON_ERROR_ENUM(_domain) NS_ENUM(NSInteger)
#endif
#define NS_ERROR_ENUM(...) __NS_ERROR_ENUM_GET_MACRO(__VA_ARGS__, __NS_NAMED_ERROR_ENUM, __NS_ANON_ERROR_ENUM)(__VA_ARGS__)
#endif

NS_ASSUME_NONNULL_BEGIN

// Kinds of database a container offers.
typedef NS_ENUM(NSInteger, CKDatabaseScope) {
    CKDatabaseScopePublic = 1,
    CKDatabaseScopePrivate = 2,
    CKDatabaseScopeShared = 3,
} NS_SWIFT_NAME(CKDatabase.Scope);

// Names of record types, record fields, subscriptions and operations are plain strings.
typedef NSString * CKRecordType;
typedef NSString * CKRecordFieldKey;
typedef NSString * CKSubscriptionID;
typedef NSString * CKOperationID;

NS_ASSUME_NONNULL_END
