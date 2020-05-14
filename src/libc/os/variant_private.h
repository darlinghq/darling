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
#include <os/availability.h>

/*! @header
 * OS Variant SPI
 *
 * Provides a mechanism to determine the currently running OS variant.
 *
 * Any of these APIs may be overridden to its non-internal behavior on a
 * device by creating on override file.  On macOS, the path of this file
 * is:
 *    /var/db/os_variant_override
 * On embedded platforms, the path of the override file is:
 *    /usr/share/misc/os_variant_override
 *
 * Individual internal behaviors can be selectively disabled (ie.
 * individual os_variant_has_internal_*() predicates can be overriden to
 * false) by writing the file with a comma- or newline-delimited list of
 * behaviors to disable.  To disable all internal behaviors, empty the file.
 *
 * There is currently no support for configuring per-subsystem overrides.
 *
 * Examples:
 *   This will disable internal diagnostics and UI on macOS:
 *     sudo sh -c 'echo "diagnostics,ui" > /var/db/os_variant_override'
 *   This will disable internal UI on iOS (assuming logged in as root):
 *     echo "ui" > /usr/share/misc/os_variant_override
 *   This will disable all internal behaviors on macOS:
 *     sudo sh -c '/bin/echo -n > /var/db/os_variant_override'
 *
 * Note that the values returned by these APIs are cached in the kernel at
 * system boot.  A reboot will be required after changing the overrides
 * before the new settings will take effect.
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
API_AVAILABLE(macosx(10.13), ios(11.0), tvos(11.0), watchos(4.0))
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
API_AVAILABLE(macosx(10.13), ios(11.0), tvos(11.0), watchos(4.0))
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
 * a manner similar to the MobileGestalt check for InternalBuild.
 *
 * @result
 * Returns true if this build has this property.  False otherwise or upon error.
 */
API_AVAILABLE(macosx(10.13), ios(11.0), tvos(11.0), watchos(4.0))
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
API_AVAILABLE(macosx(10.13), ios(11.0), tvos(11.0), watchos(4.0))
OS_EXPORT OS_WARN_RESULT
bool
os_variant_allows_internal_security_policies(const char *subsystem);

/*!
 * @function os_variant_has_factory_content
 *
 * @abstract returns whether this system has factory diagnostics content
 *
 * @description
 *
 * On macOS, this checks for a AppleFactoryVariant.plist that is present in the
 * factory diagnostics image.
 *
 * On embedded platforms, this will check for the NonUI variant.
 *
 * @result
 * Returns true if this build has this property.  False otherwise or upon error.
 */
API_AVAILABLE(macosx(10.14.4), ios(12.2), tvos(12.2), watchos(5.2))
OS_EXPORT OS_WARN_RESULT
bool
os_variant_has_factory_content(const char *subsystem);

/*!
 * @function os_variant_is_darwinos
 *
 * @abstract returns whether this system variant is a darwinOS variant
 *
 * @result
 * Returns true if this variant is a darwinOS variant.
 */
API_AVAILABLE(macosx(10.15), ios(13.0), tvos(13.0), watchos(6.0))
OS_EXPORT OS_WARN_RESULT
bool
os_variant_is_darwinos(const char *subsystem);

/*!
 * @function os_variant_uses_ephemeral_storage
 *
 * @abstract returns whether the system is booted from an ephermeral volume
 *
 * @result
 * Returns true if the system is booted with ephemeral storage for the data volume.
 */
API_AVAILABLE(macosx(10.15), ios(13.0), tvos(13.0), watchos(6.0))
OS_EXPORT OS_WARN_RESULT
bool
os_variant_uses_ephemeral_storage(const char *subsystem);

/*!
 * @function os_variant_is_recovery
 *
 * @abstract returns whether this system variant is the recovery OS.
 *
 * @description
 * On macOS, this returns whether the running environment is the BaseSystem.
 * This will be true in the installer and recovery environments.  On embedded
 * platforms, this returns whether this is the NeRD (Network Recovery on
 * Device) OS.
 *
 * @result
 * Returns true if this variant is a recoveryOS
 */
API_AVAILABLE(macosx(10.15), ios(13.0), tvos(13.0), watchos(6.0))
OS_EXPORT OS_WARN_RESULT
bool
os_variant_is_recovery(const char *subsystem);

/*!
 * @function os_variant_check
 *
 * @abstract returns whether the system is of the specified variant
 *
 * @description
 * This check checks against below known variants. False is returned if the
 * variant passed in is not in the list.
 *
 *     HasInternalContent
 *     HasInternalDiagnostics
 *     HasInternalUI
 *     AllowsInternalSecurityPolicies
 *     HasFactoryContent
 *     IsDarwinOS
 *     UsesEphemeralStorage
 *     IsRecovery
 *
 * @result
 * Returns true if the system is of the specified variant.
 */
API_AVAILABLE(macosx(10.15), ios(13.0), tvos(13.0), watchos(6.0))
OS_EXPORT OS_WARN_RESULT
bool
os_variant_check(const char *subsystem, const char *variant);

__END_DECLS

#endif // __os_variant_H__
