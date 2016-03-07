/*
 * Copyright (c) 2011-2012 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __OS_OBJECT__
#define __OS_OBJECT__

#ifdef __APPLE__
#include <Availability.h>
#endif

/*!
 * @header
 *
 * @preprocinfo
 * By default, libSystem objects such as GCD and XPC objects are declared as
 * Objective-C types when building with an Objective-C compiler. This allows
 * them to participate in ARC, in RR management by the Blocks runtime and in
 * leaks checking by the static analyzer, and enables them to be added to Cocoa
 * collections.
 *
 * NOTE: this requires explicit cancellation of dispatch sources and xpc
 *       connections whose handler blocks capture the source/connection object,
 *       resp. ensuring that such captures do not form retain cycles (e.g. by
 *       declaring the source as __weak).
 *
 * To opt-out of this default behavior, add -DOS_OBJECT_USE_OBJC=0 to your
 * compiler flags.
 *
 * This mode requires a platform with the modern Objective-C runtime, the
 * Objective-C GC compiler option to be disabled, and at least a Mac OS X 10.8
 * deployment target.
 */

#ifndef OS_OBJECT_HAVE_OBJC_SUPPORT
#if defined(__OBJC2__) && !defined(__OBJC_GC__) && ( \
		__MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_8 || \
		__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_6_0)
#define OS_OBJECT_HAVE_OBJC_SUPPORT 1
#else
#define OS_OBJECT_HAVE_OBJC_SUPPORT 0
#endif
#endif

#if OS_OBJECT_HAVE_OBJC_SUPPORT
#ifndef OS_OBJECT_USE_OBJC
#define OS_OBJECT_USE_OBJC 1
#endif
#elif defined(OS_OBJECT_USE_OBJC) && OS_OBJECT_USE_OBJC
/* Unsupported platform for OS_OBJECT_USE_OBJC=1 */
#undef OS_OBJECT_USE_OBJC
#define OS_OBJECT_USE_OBJC 0
#else
#define OS_OBJECT_USE_OBJC 0
#endif

#if OS_OBJECT_USE_OBJC
#import <Foundation/NSObject.h>
#define OS_OBJECT_CLASS(name) OS_##name
#define OS_OBJECT_DECL(name, ...) \
		@protocol OS_OBJECT_CLASS(name) __VA_ARGS__ \
		@end \
		typedef NSObject<OS_OBJECT_CLASS(name)> *name##_t
#define OS_OBJECT_DECL_SUBCLASS(name, super) \
		OS_OBJECT_DECL(name, <OS_OBJECT_CLASS(super)>)
#if defined(__has_attribute) && __has_attribute(ns_returns_retained)
#define OS_OBJECT_RETURNS_RETAINED __attribute__((__ns_returns_retained__))
#else
#define OS_OBJECT_RETURNS_RETAINED
#endif
#if defined(__has_feature) && __has_feature(objc_arc)
#define OS_OBJECT_BRIDGE __bridge
#else
#define OS_OBJECT_BRIDGE
#endif
#ifndef OS_OBJECT_USE_OBJC_RETAIN_RELEASE
#if defined(__has_feature) && __has_feature(objc_arc) || \
		defined(__clang_analyzer__)
#define OS_OBJECT_USE_OBJC_RETAIN_RELEASE 1
#else
#define OS_OBJECT_USE_OBJC_RETAIN_RELEASE 0
#endif
#endif
#else
/*! @parseOnly */
#define OS_OBJECT_RETURNS_RETAINED
/*! @parseOnly */
#define OS_OBJECT_BRIDGE
#define OS_OBJECT_USE_OBJC_RETAIN_RELEASE 0
#endif

#endif
