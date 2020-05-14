/*!
 * @header
 * TAPI-specific header to ensure project builds properly in installapi.
 */
#ifndef __DARWIN_TAPI_H
#define __DARWIN_TAPI_H

#if !DARWIN_TAPI
#error "This header is for the installapi action only"
#endif

#include <os/base.h>
#include <os/availability.h>
#include <mach/kern_return.h>
#include <mach/port.h>
#include <mach/mach_port.h>

#undef os_assert_mach
#undef os_assert_mach_port_status

// Duplicate declarations to make TAPI happy.
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
OS_EXPORT OS_NONNULL1
void
os_assert_mach(const char *op, kern_return_t kr);

API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
OS_EXPORT
void
os_assert_mach_port_status(const char *desc, mach_port_t p,
		mach_port_status_t *expected);

// TAPI and the compiler don't agree about header search paths, so if TAPI found
// our header in the SDK, and we've increased the API version, help it out.
#if DARWIN_API_VERSION < 20170407
#define DARWIN_API_AVAILABLE_20170407
#endif

#if DARWIN_API_VERSION < 20180727
#define DARWIN_API_AVAILABLE_20180727
#endif

#if DARWIN_API_VERSION < 20181020
#define DARWIN_API_AVAILABLE_20181020
#endif

#if DARWIN_API_VERSION < 20190830
#define DARWIN_API_AVAILABLE_20190830
#endif

#if DARWIN_API_VERSION < 20191015
#define DARWIN_API_AVAILABLE_20191015
#endif

#if !defined(LINKER_SET_ENTRY)
#define LINKER_SET_ENTRY(_x, _y)
#endif

#endif // __DARWIN_TAPI_H
