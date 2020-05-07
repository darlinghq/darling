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

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __OS_OBJECT_PRIVATE__
#define __OS_OBJECT_PRIVATE__

#include <sys/cdefs.h>
#include <stddef.h>
#include <os/object.h>

#ifndef __OSX_AVAILABLE_STARTING
#define __OSX_AVAILABLE_STARTING(x, y)
#endif

#if __GNUC__
#define OS_OBJECT_NOTHROW __attribute__((__nothrow__))
#define OS_OBJECT_NONNULL __attribute__((__nonnull__))
#define OS_OBJECT_WARN_RESULT __attribute__((__warn_unused_result__))
#define OS_OBJECT_MALLOC __attribute__((__malloc__))
#define OS_OBJECT_EXPORT extern __attribute__((visibility("default")))
#else
/*! @parseOnly */
#define OS_OBJECT_NOTHROW
/*! @parseOnly */
#define OS_OBJECT_NONNULL
/*! @parseOnly */
#define OS_OBJECT_WARN_RESULT
/*! @parseOnly */
#define OS_OBJECT_MALLOC
#define OS_OBJECT_EXPORT extern
#endif

#if OS_OBJECT_USE_OBJC && defined(__has_feature)
#if __has_feature(objc_arc)
#define _OS_OBJECT_OBJC_ARC 1
#else
#define _OS_OBJECT_OBJC_ARC 0
#endif
#else
#define _OS_OBJECT_OBJC_ARC 0
#endif

#define _OS_OBJECT_GLOBAL_REFCNT INT_MAX

#define _OS_OBJECT_HEADER(isa, ref_cnt, xref_cnt) \
	isa; /* must be pointer-sized */ \
	int volatile ref_cnt; \
	int volatile xref_cnt

#if OS_OBJECT_HAVE_OBJC_SUPPORT
// Must match size of compiler-generated OBJC_CLASS structure rdar://10640168
#define _OS_OBJECT_CLASS_HEADER() \
	        void *_os_obj_objc_class_t[5]
#else
#define _OS_OBJECT_CLASS_HEADER() \
	        void (*_os_obj_xref_dispose)(_os_object_t); \
	        void (*_os_obj_dispose)(_os_object_t)
#endif

#define OS_OBJECT_CLASS(name) OS_##name

#if OS_OBJECT_USE_OBJC
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
OS_OBJECT_EXPORT
@interface OS_OBJECT_CLASS(object) : NSObject
- (void)_xref_dispose;
- (void)_dispose;
@end
typedef OS_OBJECT_CLASS(object) *_os_object_t;
#define _OS_OBJECT_DECL_SUBCLASS_INTERFACE(name, super) \
	        @interface OS_OBJECT_CLASS(name) : OS_OBJECT_CLASS(super) \
	        <OS_OBJECT_CLASS(name)> \
	        @end
#else
typedef struct _os_object_s *_os_object_t;
#endif

__BEGIN_DECLS

#if !_OS_OBJECT_OBJC_ARC

    __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
OS_OBJECT_EXPORT OS_OBJECT_MALLOC OS_OBJECT_WARN_RESULT OS_OBJECT_NOTHROW
_os_object_t
_os_object_alloc(const void *cls, size_t size);

__OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_7_0)
OS_OBJECT_EXPORT OS_OBJECT_MALLOC OS_OBJECT_WARN_RESULT OS_OBJECT_NOTHROW
_os_object_t
_os_object_alloc_realized(const void *cls, size_t size);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
void _os_object_dealloc(_os_object_t object);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
_os_object_t
_os_object_retain(_os_object_t object);

__OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0)
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
_os_object_t
_os_object_retain_with_resurrect(_os_object_t obj);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
void
_os_object_release(_os_object_t object);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
_os_object_t
_os_object_retain_internal(_os_object_t object);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
OS_OBJECT_EXPORT OS_OBJECT_NONNULL OS_OBJECT_NOTHROW
void
_os_object_release_internal(_os_object_t object);

#endif // !_OS_OBJECT_OBJC_ARC

__END_DECLS

#endif
