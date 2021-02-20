//
//  si_compare.h
//  Libinfo
//
//  Copyright (c) 2011-2017 Apple Inc. All rights reserved.
//

#ifndef __SI_COMPARE_H__
#define __SI_COMPARE_H__

#include <sys/cdefs.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <os/availability.h>


__BEGIN_DECLS

/*!
 * @function si_destination_compare
 *
 * Equivalent to the si_destination_compare function, but doesn't take any routing table information
 * into account. This follows rules 6, 8 and 10 of RFC 6724 destination address selection.
 *
 * @param destination1
 * The first potential destination address.
 *
 * @param destination2
 * The second potential destination address.
 *
 * @result
 * Returns -1 if destination1 is less desirable than destination 2. Returns 0 if destination1
 * is equally as desirable as destination 2. Returns 1 if destination 1 is more desirable than
 * destination 2.
 */
API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0))
int si_destination_compare(const struct sockaddr *dst1, int dst1ifindex,
						   const struct sockaddr *dst2, int dst2ifindex,
						   bool statistics);

/*!
 * @function si_destination_compare_no_dependencies
 *
 * Equivalent to the si_destination_compare function, but doesn't take any routing table information
 * into account. This follows rules 6, 8 and 10 of RFC 6724 destination address selection.
 *
 * @param destination1
 * The first potential destination address.
 *
 * @param destination2
 * The second potential destination address.
 *
 * @result
 * Returns -1 if destination1 is less desirable than destination 2. Returns 0 if destination1
 * is equally as desirable as destination 2. Returns 1 if destination 1 is more desirable than
 * destination 2.
 */
API_AVAILABLE(macos(10.14), ios(12.0), watchos(5.0), tvos(12.0))
int
si_destination_compare_no_dependencies(const struct sockaddr *destination1,
									   const struct sockaddr *destination2);

__END_DECLS

#endif // __SI_COMPARE_H__
