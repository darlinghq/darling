#ifndef _NETINET_DHCP_OPTIONS_H
#define _NETINET_DHCP_OPTIONS_H
#include <sys/appleapiopts.h>
/*
 * Copyright (c) 1999-2016 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

/*
 * dhcp_options.h
 * - routines to parse and access dhcp options
 */

/*
 * Modification History
 *
 * March 15, 2002	Dieter Siegmund (dieter@apple)
 * - imported from bootp project
 */
#include <mach/boolean.h>
#include <netinet/dhcp.h>


/* overloaded option values */
#define DHCP_OVERLOAD_FILE      1
#define DHCP_OVERLOAD_SNAME     2
#define DHCP_OVERLOAD_BOTH      3

/*
 * DHCP_OPTION_SIZE_MAX
 * - the largest size that an option can be (limited to an 8-bit quantity)
 */
#define DHCP_OPTION_SIZE_MAX    255

#define DHCP_TAG_OFFSET         0
#define DHCP_LEN_OFFSET         1
#define DHCP_OPTION_OFFSET      2



#define RFC_OPTIONS_MAGIC               { 99, 130, 83, 99 }
#define RFC_MAGIC_SIZE                  4 /* bytes */

enum {
	/* rfc 1497 vendor extensions: 0..18, 255 */
	dhcptag_pad_e                       = 0,
	dhcptag_end_e                       = 255,
	dhcptag_subnet_mask_e               = 1,
	dhcptag_time_offset_e               = 2,
	dhcptag_router_e                    = 3,
	dhcptag_time_server_e               = 4,
	dhcptag_name_server_e               = 5,
	dhcptag_domain_name_server_e        = 6,
	dhcptag_log_server_e                = 7,
	dhcptag_cookie_server_e             = 8,
	dhcptag_lpr_server_e                = 9,
	dhcptag_impress_server_e            = 10,
	dhcptag_resource_location_server_e  = 11,
	dhcptag_host_name_e                 = 12,
	dhcptag_boot_file_size_e            = 13,
	dhcptag_merit_dump_file_e           = 14,
	dhcptag_domain_name_e               = 15,
	dhcptag_swap_server_e               = 16,
	dhcptag_root_path_e                 = 17,
	dhcptag_extensions_path_e           = 18,

	/* ip layer parameters per host: 19..25 */
	dhcptag_ip_forwarding_e             = 19,
	dhcptag_non_local_source_routing_e  = 20,
	dhcptag_policy_filter_e             = 21,
	dhcptag_max_dgram_reassembly_size_e = 22,
	dhcptag_default_ip_time_to_live_e   = 23,
	dhcptag_path_mtu_aging_timeout_e    = 24,
	dhcptag_path_mtu_plateau_table_e    = 25,

	/* ip layer parameters per interface: 26..33 */
	dhcptag_interface_mtu_e             = 26,
	dhcptag_all_subnets_local_e         = 27,
	dhcptag_broadcast_address_e         = 28,
	dhcptag_perform_mask_discovery_e    = 29,
	dhcptag_mask_supplier_e             = 30,
	dhcptag_perform_router_discovery_e  = 31,
	dhcptag_router_solicitation_address_e       = 32,
	dhcptag_static_route_e              = 33,
	dhcptag_trailer_encapsulation_e     = 34,
	dhcptag_arp_cache_timeout_e         = 35,
	dhcptag_ethernet_encapsulation_e    = 36,

	/* tcp parameters: 37..39 */
	dhcptag_default_ttl_e               = 37,
	dhcptag_keepalive_interval_e        = 38,
	dhcptag_keepalive_garbage_e         = 39,

	/* application & service parameters: 40..49, 64, 65, 68..76, 78, 79, 95 */
	dhcptag_nis_domain_e                = 40,
	dhcptag_nis_servers_e               = 41,
	dhcptag_network_time_protocol_servers_e     = 42,
	dhcptag_vendor_specific_e           = 43,
	dhcptag_nb_over_tcpip_name_server_e = 44,
	dhcptag_nb_over_tcpip_dgram_dist_server_e   = 45,
	dhcptag_nb_over_tcpip_node_type_e   = 46,
	dhcptag_nb_over_tcpip_scope_e       = 47,
	dhcptag_x_windows_font_server_e     = 48,
	dhcptag_x_windows_display_manager_e = 49,
	dhcptag_nis_plus_domain_e           = 64,
	dhcptag_nis_plus_servers_e          = 65,
	dhcptag_mobile_ip_home_agent_e      = 68,
	dhcptag_smtp_server_e               = 69,
	dhcptag_pop3_server_e               = 70,
	dhcptag_nntp_server_e               = 71,
	dhcptag_default_www_server_e        = 72,
	dhcptag_default_finger_server_e     = 73,
	dhcptag_default_irc_server_e        = 74,
	dhcptag_streettalk_server_e         = 75,
	dhcptag_stda_server_e               = 76,
	dhcptag_slp_directory_agent_e       = 78,
	dhcptag_slp_service_scope_e         = 79,
	dhcptag_ldap_url_e                  = 95,

	/* dhcp-specific extensions: 50..61, 66, 67 */
	dhcptag_requested_ip_address_e      = 50,
	dhcptag_lease_time_e                = 51,
	dhcptag_option_overload_e           = 52,
	dhcptag_dhcp_message_type_e         = 53,
	dhcptag_server_identifier_e         = 54,
	dhcptag_parameter_request_list_e    = 55,
	dhcptag_message_e                   = 56,
	dhcptag_max_dhcp_message_size_e     = 57,
	dhcptag_renewal_t1_time_value_e     = 58,
	dhcptag_rebinding_t2_time_value_e   = 59,
	dhcptag_vendor_class_identifier_e   = 60,
	dhcptag_client_identifier_e         = 61,
	dhcptag_tftp_server_name_e          = 66,
	dhcptag_bootfile_name_e             = 67,

	/* netinfo parent tags: 112, 113 */
	dhcptag_netinfo_server_address_e    = 112,
	dhcptag_netinfo_server_tag_e        = 113,

	/* ad-hoc network disable option */
	dhcptag_auto_configure_e            = 116,
};
typedef uint8_t dhcptag_t;

/*
 * Module: dhcpol (dhcp options list)
 *
 * Purpose:
 *   Routines to parse and retrieve dhcp options.
 */

typedef struct {
	const void * *      array;/* malloc'd array of pointers */
	int                 size;/* number of elements in array */
	int                 count;/* number of occupied elements */
} ptrlist_t;

typedef ptrlist_t dhcpol_t;

#ifdef BSD_KERNEL_PRIVATE

void                    dhcpol_init(dhcpol_t * list);
void                    dhcpol_free(dhcpol_t * list);
int                     dhcpol_count(dhcpol_t * list);
boolean_t               dhcpol_add(dhcpol_t * list, const void * element);
const void *            dhcpol_element(dhcpol_t * list, int i);
boolean_t               dhcpol_concat(dhcpol_t * list, dhcpol_t * extra);
boolean_t               dhcpol_parse_buffer(dhcpol_t * list,
    const void * buffer,
    int length);
const void *            dhcpol_find(dhcpol_t * list, int tag, int * len_p,
    int * start);
boolean_t               dhcpol_parse_packet(dhcpol_t * options,
    const struct dhcp * pkt, int len);

#endif /* BSD_KERNEL_PRIVATE */
#endif /* _NETINET_DHCP_OPTIONS_H */
