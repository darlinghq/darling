/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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

#ifndef __OS_VARIANT_H__
#define __OS_VARIANT_H__

#include <stdbool.h>

#include <os/base.h>

/*! @header
 * OS Variant SPI
 *
 * Provides a mechanism to determine the currently running OS variant.
 *
 * Any of these APIs may be overridden to its non-internal behavior on a
 * device by creating on override file.  On macOS, this file is placed
 * at:
 *    /var/db/os_variant_override
 * On embedded platforms, this file is placed at:
 *    /usr/share/misc/os_variant_override
 *
 * Individual internal behaviors can be selectively disabled (ie.
 * individual os_variant_has_internal_*() predicates can be overriden to
 * false) by writing the file with a comma- or newline-delimited list of
 * names to disable.  To disable all internal behaviors, empty the file.
 *
 * Each of these functions takes a constant string argument for the requesting
 * subsystem.  This should be a reverse-DNS string describing the subsystem
 * performing the check.  This may be used in the future for auditing and
 * selective overriding of checks.
 *
 */

__BEGIN_DECLS

/*!
 * @function os_variant_has_internal_content
 *
 * @abstract returns whether this system variant has internal content installed
 * ("content")
 *
 * @result
 * Returns true if this build has this property.  False otherwise or upon error.
 */
OS_EXPORT OS_WARN_RESULT
bool
os_variant_has_internal_content(const char *subsystem);

/*!
 * @function os_variant_has_internal_diagnostics
 *
 * @abstract returns whether this system variant has internal diagnostics
 * enabled ("diagnostics")
 *
 * @description
 *
 * Internal diagnostics include behaviors that emit extra diagnostic or
 * debugging information when an error occurs.
 *
 * On macOS, this check will look for presence of AppleInternal content or the
 * AppleInternalDiagnostics profile to be installed.
 *
 * On embedded platforms, this check will look for an internal install variant
 * in a manner similar to the MobileGestalt check for InternalBuild.
 *
 * @result
 * Returns true if this build has this property.  False otherwise or upon error.
 */
OS_EXPORT OS_WARN_RESULT
bool
os_variant_has_internal_diagnostics(const char *subsystem);

/*!
 * @function os_variant_has_internal_ui
 *
 * @abstract returns whether this system variant has internal UI visible ("ui")
 *
 * @description
 *
 * Internal UI includes debug menus and internal settings.
 *
 * On macOS, this will check for the presence of AppleInternal content.  On
 * embedded platforms, this check will look for an internal install variant in
 * a manor similar to the MobileGestalt check for InternalBuild.
 *
 * @result
 * Returns true if this build has this property.  False otherwise or upon error.
 */
OS_EXPORT OS_WARN_RESULT
bool
os_variant_has_internal_ui(const char *subsystem);

/*!
 * @function os_variant_allows_internal_security_policies
 *
 * @abstract returns whether this system variant allows internal security policies
 * ("security")
 *
 * @description
 *
 * On macOS, this will check the CSR status for whether AppleInternal policies
 * are enabled.
 *
 * On embedded platforms, this will check for a build/device combination that
 * allows for removal of codesigning and debugging restrictions.  This usually
 * returns whether the hardware is development fused and may return true on
 * such hardware even if a customer build is installed.
 *
 * n.b. The result of this API should /not/ be used to automatically enable
 * relaxed security policies, only to signal that other mechanisms to enable
 * them are allowed, e.g. a "defaults write".
 *
 * @result
 * Returns true if this build has this property.  False otherwise or upon error.
 */
OS_EXPORT OS_WARN_RESULT
bool
os_variant_allows_internal_security_policies(const char *subsystem);

__END_DECLS

#endif // __os_variant_H__
