/*
 * Copyright (c) 2009-2011 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#import <CoreFoundation/CFBase.h> // CF_EXPORT

#if !defined(__ODCONSTANTS_H)
#define __ODCONSTANTS_H

/*!
    @const      kODSessionProxyAddress
    @abstract   the address to connect to via proxy, used when making the options dictionary
    @discussion the address to connect to via proxy, used when making the options dictionary
*/
CF_EXPORT
const CFStringRef kODSessionProxyAddress __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @const      kODSessionProxyPort
    @abstract   the port to connect to via proxy, used when making the options dictionary
    @discussion the port to connect to via proxy, used when making the options dictionary.  This parameter
                is optional and should not be passed normally.
*/
CF_EXPORT
const CFStringRef kODSessionProxyPort __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @const      kODSessionProxyUsername
    @abstract   the username to connect with via proxy, used when making the options dictionary
    @discussion the username to connect with via proxy, used when making the options dictionary
*/
CF_EXPORT
const CFStringRef kODSessionProxyUsername __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @const      kODSessionProxyPassword
    @abstract   the password to connect with via proxy, used when making the options dictionary
    @discussion the password to connect with via proxy, used when making the options dictionary
*/
CF_EXPORT
const CFStringRef kODSessionProxyPassword __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
 @const      kODModuleConfigOptionQueryTimeout
 @abstract   number of seconds before a query times out
 @discussion number of seconds before a query times out
 */
CF_EXPORT
const CFStringRef kODModuleConfigOptionQueryTimeout __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);
/*!
 @const      kODModuleConfigOptionConnectionSetupTimeout
 @abstract   number of seconds before a connection attempt times out
 @discussion number of seconds before a connection attempt times out
 */
CF_EXPORT
const CFStringRef kODModuleConfigOptionConnectionSetupTimeout __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);
/*!
 @const      kODModuleConfigOptionConnectionIdleDisconnect
 @abstract   number of seconds before a connection is idle disconnected
 @discussion number of seconds before a connection is idle disconnected
 */
CF_EXPORT
const CFStringRef kODModuleConfigOptionConnectionIdleDisconnect __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);
/*!
 @const      kODModuleConfigOptionPacketSigning
 @abstract   enable or disable packet signing
 @discussion number of seconds before a query times out
 */
CF_EXPORT
const CFStringRef kODModuleConfigOptionPacketSigning __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);
/*!
 @const      kODModuleConfigOptionPacketEncryption
 @abstract   enable or disable packet encryption
 @discussion enable or disable packet encryption
 */
CF_EXPORT
const CFStringRef kODModuleConfigOptionPacketEncryption __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
 @const      kODModuleConfigOptionManInTheMiddle
 @abstract   enable or disable man-in-middle countermeasures
 @discussion enable or disable man-in-middle countermeasures
 */
CF_EXPORT
const CFStringRef kODModuleConfigOptionManInTheMiddle __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
	@enum		ODNodeType
	@abstract   Types of nodes that can be opened
	@discussion Various types of nodes that can be opened.
	@constant   kODNodeTypeAuthentication is a node type commonly used for all authentications or record lookups
	@constant   kODNodeTypeContacts is a node type commonly used for applications that deal with contact data
	@constant   kODNodeTypeNetwork is a node type used for looking for network resource type data
	@constant   kODNodeTypeLocalNodes is a node type that specifically looks at the local directory
	@constant   kODNodeTypeConfigure is a node type that refers to the configuration node within DS
*/
enum
{
	kODNodeTypeAuthentication			= 0x2201,
	kODNodeTypeContacts					= 0x2204,
	kODNodeTypeNetwork					= 0x2205, /* deprecated, behaves identically to kODNodeTypeAuthentication */
	
	kODNodeTypeLocalNodes				= 0x2200,
	kODNodeTypeConfigure   				= 0x2202
};
typedef uint32_t ODNodeType;

/*!
    @const      kODNodeOptionsQuerySkippedSubnode
    @abstract   Used with nodes that support options.
    @discussion Passed in an options dictionary where the value is a boolean (kCFBooleanTrue/kCFBooleanFalse).
                If a node supports the option it will notify the client when a subnode is skipped during queries.
                Supported options are listed in node details under kODAttributeTypeNodeOptions.  Node may fail to
                open if unsupported options are used.
 */
CF_EXPORT
const CFStringRef kODNodeOptionsQuerySkippedSubnode __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_NA);

/*!
	@enum   	ODMatchType
	@abstract   Are types of matching types used for doing searches.  Each type is self explanatory based on the name.
	@constant	kODMatchAny is used to search for any records (typically passed with nil search value)
	@constant	kODMatchEqualTo is searching values that are equal to the provided value
	@constant	kODMatchBeginsWith is searching values that begin with the provided value
	@constant	kODMatchContains is searching values that contain the provided value
	@constant	kODMatchEndsWith is searching values that end with the provided value
	@constant	kODMatchGreaterThan is searching values greater than the provided value
	@constant	kODMatchLessThan is searching values less than the provided value
 */
enum
{
	kODMatchAny						= 0x0001,
	
	kODMatchEqualTo					= 0x2001,
	kODMatchBeginsWith				= 0x2002,
	kODMatchContains				= 0x2004,
	kODMatchEndsWith				= 0x2003,
	
	kODMatchInsensitiveEqualTo    __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_6, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "case matching is defined by attribute schema")		= 0x2101,
	kODMatchInsensitiveBeginsWith __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_6, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "case matching is defined by attribute schema")		= 0x2102,
	kODMatchInsensitiveContains   __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_6, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "case matching is defined by attribute schema")		= 0x2104,
	kODMatchInsensitiveEndsWith   __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_6, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "case matching is defined by attribute schema")		= 0x2103,
	
	kODMatchGreaterThan				= 0x2006,
	kODMatchLessThan				= 0x2007,
};
typedef uint32_t ODMatchType;

/*!
	@typedef	ODRecordType
	@abstract   is used to define a specific record type
	@discussion is used to define a specific record type common to DirectoryService.
				Records types are strings that have a prefix:
 
					Standard records	- "dsRecTypeStandard:"
					Native to the node	- "dsRecTypeNative:"
 
				The most commonly used Standard records are contained in this header.
 
				Note:  CFStringRef can be use interchangeably with ODRecordType for ease
				of use.
*/
#ifdef __OBJC__
#include <Foundation/Foundation.h>
typedef NSString *ODRecordType;
#else
typedef CFStringRef ODRecordType;
#endif

/*!
	@typedef	ODAttributeType
	@abstract   is used to define a specific attribute type
	@discussion is used to define a specific attribute type common to DirectoryService.
				Attribute types are strings that have a prefix:
 
					Standard attributes	- "dsAttrTypeStandard:"
					Native to the node	- "dsAttrTypeNative:"
 
				The most commonly used Standard attributes are contained in this header.
 
				Note:  CFStringRef can be use interchangeably with ODAttributeType for ease
				of use.
*/
#ifdef __OBJC__
typedef NSString *ODAttributeType;
#else
typedef CFStringRef ODAttributeType;
#endif

/*!
	@typedef	ODAuthenticationType
	@abstract   is used to define a specific credential method type
	@discussion is used to define a specific credential method type common to DirectoryService.
				Authentication types are strings that have a prefix:
 
					Standard attributes	- "dsAuthMethodStandard:"
					Native to the node	- "dsAuthMethodNative:"
 
				The most commonly used Standard authentications are contained in this header.
 
				Note:  CFStringRef can be use interchangeably with ODAuthenticationType for ease
				of use.
*/
#ifdef __OBJC__
typedef NSString *ODAuthenticationType;
#else
typedef CFStringRef ODAuthenticationType;
#endif

/*!
	@typedef	ODPolicyType
	@abstract   is used to modify policies on nodes or records
	@discussion is used to modify policies on nodes or records
*/
#ifdef __OBJC__
typedef NSString *ODPolicyType;
#else
typedef CFStringRef ODPolicyType;
#endif

/*!
    @typedef    ODErrorUserInfoKeyType
    @abstract   Type for any additional keys in userInfo dictionary in NS/CFError
 */
#ifdef __OBJC__
typedef NSString *ODErrorUserInfoKeyType;
#else
typedef CFStringRef ODErrorUserInfoKeyType;
#endif

// Compatibility
typedef ODRecordType _ODRecordType;
typedef ODAttributeType _ODAttributeType;
typedef ODAuthenticationType _ODAuthenticationType;

#pragma mark Record Types

/*!
    @const		kODRecordTypeAttributeTypes
    @abstract   Identifies records that represent each possible attribute type.
    @discussion Identifies records that represent each possible attribute type.
*/
CF_EXPORT
const ODRecordType kODRecordTypeAttributeTypes;

/*!
    @const		kODRecordTypeAFPServer
    @abstract   Record type of AFP server records.
    @discussion Record type of AFP server records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeAFPServer;

/*!
    @const		kODRecordTypeAliases
    @abstract   Used to represent alias records.
    @discussion Used to represent alias records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeAliases;

/*!
    @const		kODRecordTypeAugments
    @abstract   Used to store augmented record data.
    @discussion Used to store augmented record data.
*/
CF_EXPORT
const ODRecordType kODRecordTypeAugments;

/*!
    @const		kODRecordTypeAutomount
    @abstract   Used to store automount record data.
    @discussion Used to store automount record data.
*/
CF_EXPORT
const ODRecordType kODRecordTypeAutomount;

/*!
    @const		kODRecordTypeAutomountMap
    @abstract   Used to store automountMap record data.
    @discussion Used to store automountMap record data.
*/
CF_EXPORT
const ODRecordType kODRecordTypeAutomountMap;

/*!
    @const		kODRecordTypeAutoServerSetup
    @abstract   Used to discover automated server setup information.
    @discussion Used to discover automated server setup information.
*/
CF_EXPORT
const ODRecordType kODRecordTypeAutoServerSetup;

/*!
    @const		kODRecordTypeBootp
    @abstract   Record in the local node for storing bootp info.
    @discussion Record in the local node for storing bootp info.
*/
CF_EXPORT
const ODRecordType kODRecordTypeBootp;

/*!
    @const		kODRecordTypeCertificateAuthorities
    @abstract   Record type that contains certificate authority information.
    @discussion Record type that contains certificate authority information.
*/
CF_EXPORT
const ODRecordType kODRecordTypeCertificateAuthorities;

/*!
    @const		kODRecordTypeComputerLists
    @abstract   Identifies computer list records.
    @discussion Identifies computer list records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeComputerLists;

/*!
    @const		kODRecordTypeComputerGroups
    @abstract   Identifies computer group records.
    @discussion Identifies computer group records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeComputerGroups;

/*!
    @const		kODRecordTypeComputers
    @abstract   Identifies computer records.
    @discussion Identifies computer records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeComputers;

/*!
    @const		kODRecordTypeConfiguration
    @abstract   Identifies configuration records.
    @discussion Identifies configuration records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeConfiguration;

/*!
    @const		kODRecordTypeEthernets
    @abstract   Record in the node for storing ethernets.
    @discussion Record in the node for storing ethernets.
*/
CF_EXPORT
const ODRecordType kODRecordTypeEthernets;

/*!
    @const		kODRecordTypeFileMakerServers
	@abstract   FileMaker servers record type. 
	@discussion FileMaker servers record type that describes available FileMaker servers, 
				used for service discovery.
*/
CF_EXPORT
const ODRecordType kODRecordTypeFileMakerServers;

/*!
    @const		kODRecordTypeFTPServer
    @abstract   Identifies ftp server records.
    @discussion Identifies ftp server records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeFTPServer;

/*!
    @const		kODRecordTypeGroups
    @abstract   Identifies group records.
    @discussion Identifies group records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeGroups;

/*!
    @const		kODRecordTypeHostServices
    @abstract   Record in the local node for storing host services.
    @discussion Record in the local node for storing host services.
*/
CF_EXPORT
const ODRecordType kODRecordTypeHostServices;

/*!
    @const		kODRecordTypeHosts
    @abstract   Identifies host records.
    @discussion Identifies host records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeHosts;

/*!
    @const		kODRecordTypeLDAPServer
    @abstract   Identifies LDAP server records.
    @discussion Identifies LDAP server records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeLDAPServer;

/*!
    @const		kODRecordTypeLocations
    @abstract   Location record type.
    @discussion Location record type.
*/
CF_EXPORT
const ODRecordType kODRecordTypeLocations;

/*!
    @const		kODRecordTypeMounts
    @abstract   Identifies mount records.
    @discussion Identifies mount records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeMounts;

/*!
    @const		kODRecordTypeNFS
    @abstract   Identifies NFS records.
    @discussion Identifies NFS records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeNFS;

/*!
    @const		kODRecordTypeNetDomains
    @abstract   Record in the local node for storing net domains.
    @discussion Record in the local node for storing net domains.
*/
CF_EXPORT
const ODRecordType kODRecordTypeNetDomains;

/*!
    @const		kODRecordTypeNetGroups
    @abstract   Record in the local node for storing net groups.
    @discussion Record in the local node for storing net groups.
*/
CF_EXPORT
const ODRecordType kODRecordTypeNetGroups;

/*!
    @const		kODRecordTypeNetworks
    @abstract   Identifies network records.
    @discussion Identifies network records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeNetworks;

/*!
    @const		kODRecordTypePeople
    @abstract   Record type that contains "People" records used for contact information.
    @discussion Record type that contains "People" records used for contact information.
*/
CF_EXPORT
const ODRecordType kODRecordTypePeople;

/*!
    @const		kODRecordTypePresetComputers
    @abstract   The computer record type used for presets in record creation.
    @discussion The computer record type used for presets in record creation.
*/
CF_EXPORT
const ODRecordType kODRecordTypePresetComputers;

/*!
    @const		kODRecordTypePresetComputerGroups
    @abstract   The computer group record type used for presets in record creation.
    @discussion The computer group record type used for presets in record creation.
*/
CF_EXPORT
const ODRecordType kODRecordTypePresetComputerGroups;

/*!
    @const		kODRecordTypePresetComputerLists
    @abstract   The computer list record type used for presets in record creation.
    @discussion The computer list record type used for presets in record creation.
*/
CF_EXPORT
const ODRecordType kODRecordTypePresetComputerLists;

/*!
    @const		kODRecordTypePresetGroups
    @abstract   The group record type used for presets in record creation.
    @discussion The group record type used for presets in record creation.
*/
CF_EXPORT
const ODRecordType kODRecordTypePresetGroups;

/*!
    @const		kODRecordTypePresetUsers
    @abstract   The user record type used for presets in record creation.
    @discussion The user record type used for presets in record creation.
*/
CF_EXPORT
const ODRecordType kODRecordTypePresetUsers;

/*!
    @const		kODRecordTypePrintService
    @abstract   Identifies print service records.
    @discussion Identifies print service records.
*/
CF_EXPORT
const ODRecordType kODRecordTypePrintService;

/*!
    @const		kODRecordTypePrintServiceUser
    @abstract   Record in the local node for storing quota usage for a user.
    @discussion Record in the local node for storing quota usage for a user.
*/
CF_EXPORT
const ODRecordType kODRecordTypePrintServiceUser;

/*!
    @const		kODRecordTypePrinters
    @abstract   Identifies printer records.
    @discussion Identifies printer records.
*/
CF_EXPORT
const ODRecordType kODRecordTypePrinters;

/*!
    @const		kODRecordTypeProtocols
    @abstract   Identifies protocol records.
    @discussion Identifies protocol records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeProtocols;

/*!
    @const		kODRecordTypeQTSServer
    @abstract   Identifies quicktime streaming server records.
    @discussion Identifies quicktime streaming server records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeQTSServer;

/*!
    @const		kODRecordTypeQueryInformation
    @abstract   Special query information record
    @discussion Specifically to return query information inline such as skipped nodes
 */
CF_EXPORT
const ODRecordType kODRecordTypeQueryInformation;

/*!
    @const		kODRecordTypeRecordTypes
    @abstract   Identifies records that represent each possible record type.
    @discussion Identifies records that represent each possible record type.
*/
CF_EXPORT
const ODRecordType kODRecordTypeRecordTypes;

/*!
    @const		kODRecordTypeResources
    @abstract   Identifies resources used in group services.
    @discussion Identifies resources used in group services.
*/
CF_EXPORT
const ODRecordType kODRecordTypeResources;

/*!
    @const		kODRecordTypeRPC
    @abstract   Identifies remote procedure call records.
    @discussion Identifies remote procedure call records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeRPC;

/*!
    @const		kODRecordTypeSMBServer
    @abstract   Identifies SMB server records.
    @discussion Identifies SMB server records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeSMBServer;

/*!
    @const		kODRecordTypeServer
    @abstract   Identifies generic server records.
    @discussion Identifies generic server records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeServer;

/*!
    @const		kODRecordTypeServices
    @abstract   Identifies directory based service records.
    @discussion Identifies directory based service records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeServices;

/*!
    @const		kODRecordTypeSharePoints
    @abstract   Share point record type.
    @discussion Share point record type.
*/
CF_EXPORT
const ODRecordType kODRecordTypeSharePoints;

/*!
    @const		kODRecordTypeUsers
    @abstract   Identifies user records.
    @discussion Identifies user records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeUsers;

/*!
    @const		kODRecordTypeWebServer
    @abstract   Identifies web server records.
    @discussion Identifies web server records.
*/
CF_EXPORT
const ODRecordType kODRecordTypeWebServer;

#pragma mark Attribute Types

/*!
    @const		kODAttributeTypeAllAttributes
    @abstract   Used in requesting all attribute types in a search.
    @discussion Used in requesting all attribute types in a search.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAllAttributes;

/*!
    @const		kODAttributeTypeStandardOnly
    @abstract   Used in requesting all standard attribute types in a query.
    @discussion Used in requesting all standard attribute types in a query.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeStandardOnly;

/*!
    @const		kODAttributeTypeNativeOnly
    @abstract   Used in requesting all native attribute types in a search.
    @discussion Used in requesting all native attribute types in a search.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNativeOnly;

/*!
    @const		kODAttributeTypeMetaAmbiguousName
    @abstract   Used to disambiguate a provided name.
    @discussion Searches a configured list of attributes; by default: RecordName, FullName, EMailAddress.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMetaAmbiguousName;

/*!
    @const		kODAttributeTypeMetaAugmentedAttributes
    @abstract   Attributes that have been augmented.
    @discussion Attributes that have been augmented.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMetaAugmentedAttributes;

/*!
    @const		kODAttributeTypeMetaRecordName
    @abstract   Native record name.
    @discussion Native record name, e.g. LDAP DN
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMetaRecordName;

/*!
    @const		kODAttributeTypeAdminLimits
    @abstract   XML plist indicating what an admin user can edit.
    @discussion XML plist indicating what an admin user can edit. Found in kODRecordTypeUsers records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAdminLimits;

/*!
	@const		kODAttributeTypeAltSecurityIdentities
	@abstract	Used to store alternate identities for the record
	@discussion Used to store alternate identities for the record. Values will have standardized form as
				specified by Microsoft LDAP schema (1.2.840.113556.1.4.867).

 				Kerberos:user\@REALM
 */
CF_EXPORT
const ODAttributeType kODAttributeTypeAltSecurityIdentities __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @const		kODAttributeTypeAuthenticationHint
    @abstract   Used to identify the authentication hint phrase.
    @discussion Used to identify the authentication hint phrase.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAuthenticationHint;

/*!
    @const		kODAttributeTypeAllTypes
    @abstract   Used to indicated recommended attribute types for a record type in the Config node.
    @discussion Used to indicated recommended attribute types for a record type in the Config node.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAllTypes;

/*!
    @const		kODAttributeTypeAuthorityRevocationList
    @abstract   Attribute containing the binary of the authority revocation list
	@discussion Attribute containing the binary of the authority revocation list.
				A certificate revocation list that defines certificate authority certificates
				which are no longer trusted.  No user certificates are included in this list.
				Usually found in kODRecordTypeCertificateAuthorities records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAuthorityRevocationList;

/*!
    @const		kODAttributeTypeBirthday
    @abstract   Single-valued attribute that defines the user's birthday.
	@discussion Single-valued attribute that defines the user's birthday.
				Format is x.208 standard YYYYMMDDHHMMSSZ which we will require as GMT time.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeBirthday;

/*!
    @const		kODAttributeTypeCACertificate
	@abstract   Attribute containing the binary of the certificate of a certificate authority.
	@discussion Attribute containing the binary of the certificate of a certificate authority.
				Its corresponding private key is used to sign certificates.
				Usually found in kODRecordTypeCertificateAuthority records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeCACertificate;

/*!
    @const		kODAttributeTypeCapacity
    @abstract   Attribute type for the capacity of a resource. 
	@discussion Attribute type for the capacity of a resource. 
				found in resource records (kODRecordTypeResources). 
					Example: 50
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeCapacity;

/*!
    @const		kODAttributeTypeCertificateRevocationList
    @abstract   Attribute containing the binary of the certificate revocation list.
	@discussion Attribute containing the binary of the certificate revocation list.
				This is a list of certificates which are no longer trusted.
				Usually found in kODRecordTypeCertificateAuthority records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeCertificateRevocationList;

/*!
    @const		kODAttributeTypeComment
    @abstract   Attribute used for unformatted comment.
    @discussion Attribute used for unformatted comment.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeComment;

/*!
    @const		kODAttributeTypeContactGUID
	@abstract   Attribute type for the contact GUID of a group. 
	@discussion Attribute type for the contact GUID of a group. 
				Usually found in group records (kODRecordTypeGroups)
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeContactGUID;

/*!
    @const		kODAttributeTypeContactPerson
	@abstract   Attribute type for the contact person of the machine. 
	@discussion Attribute type for the contact person of the machine. 
				Found in host or machine records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeContactPerson;

/*!
    @const		kODAttributeTypeCreationTimestamp
	@abstract   Attribute showing date/time of record creation.
	@discussion Attribute showing date/time of record creation.
				Format is x.208 standard YYYYMMDDHHMMSSZ which should be GMT time.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeCreationTimestamp;

/*!
    @const		kODAttributeTypeCrossCertificatePair
	@abstract   Attribute containing the binary of a pair of certificates which 
				verify each other.
	@discussion Attribute containing the binary of a pair of certificates which 
				verify each other.  Both certificates have the same level of authority.
				Usually found in kODRecordTypeCertificateAuthority records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeCrossCertificatePair;

/*!
    @const		kODAttributeTypeDataStamp
    @abstract   Attribute for checksum/meta data
    @discussion Attribute for checksum/meta data
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeDataStamp;

/*!
    @const		kODAttributeTypeFullName
    @abstract   Full name of a record
    @discussion Full name of a record (e.g., "John Doe", "John Doe's Computer")
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeFullName;

/*!
    @const		kODAttributeTypeDNSDomain
    @abstract   DNS Resolver domain attribute.
    @discussion DNS Resolver domain attribute.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeDNSDomain;

/*!
    @const		kODAttributeTypeDNSNameServer
    @abstract   DNS Resolver nameserver attribute.
    @discussion DNS Resolver nameserver attribute.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeDNSNameServer;

/*!
    @const		kODAttributeTypeENetAddress
	@abstract   Attribute for hardware Ethernet address (MAC address).
	@discussion Attribute for hardware Ethernet address (MAC address).
				Found in computer records (kODRecordTypeComputers).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeENetAddress;

/*!
    @const		kODAttributeTypeExpire
    @abstract   Used for expiration date or time depending on association.
    @discussion Used for expiration date or time depending on association.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeExpire;

/*!
    @const		kODAttributeTypeFirstName
    @abstract   Used for first name of user or person record.
    @discussion Used for first name of user or person record.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeFirstName;

/*!
    @const		kODAttributeTypeGUID
	@abstract   Used for 36 character (128 bit) unique ID.
	@discussion Used for 36 character (128 bit) unique ID. An example value is
				"A579E95E-CDFE-4EBC-B7E7-F2158562170F". The standard format contains
				32 uppercase hex characters and four hyphen characters.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeGUID;

/*!
    @const      kODAttributeTypeHardwareUUID
    @abstract   Used to store hardware UUID in string form
    @discussion Used to store hardware UUID in string form for a record.  Typically found in
                kODRecordTypeComputers.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeHardwareUUID __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @const		kODAttributeTypeHomeDirectoryQuota
	@abstract   Represents the allowed usage for a user's home directory in bytes.
	@discussion Represents the allowed usage for a user's home directory in bytes.
				Found in user records (kODRecordTypeUsers).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeHomeDirectoryQuota;

/*!
    @const		kODAttributeTypeHomeDirectorySoftQuota
	@abstract   Used to define home directory size limit in bytes when user is notified
				that the hard limit is approaching.
	@discussion Used to define home directory size limit in bytes when user is notified
				that the hard limit is approaching.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeHomeDirectorySoftQuota;

/*!
    @const		kODAttributeTypeHomeLocOwner
	@abstract   Represents the owner of a workgroup's shared home directory.
	@discussion Represents the owner of a workgroup's shared home directory.
				Typically found in kODRecordTypeGroups records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeHomeLocOwner;

/*!
    @const		kODAttributeTypeInternetAlias
    @abstract   Used to track internet alias.
    @discussion Used to track internet alias.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeInternetAlias;

/*!
    @const		kODAttributeTypeKDCConfigData
    @abstract   Contents of the kdc.conf file.
    @discussion Contents of the kdc.conf file.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeKDCConfigData;

/*!
	@const		kODAttributeTypeKerberosServices
	@abstract	This is used to store the principals in host records (i.e., "host", "vnc", etc.)
	@discussion	This is used to store the principals in host records (i.e., "host", "vnc", etc.)
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeKerberosServices __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
    @const		kODAttributeTypeLastName
    @abstract   Used for the last name of user or person record.
    @discussion Used for the last name of user or person record.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeLastName;

/*!
    @const		kODAttributeTypeLDAPSearchBaseSuffix
    @abstract   Search base suffix for a LDAP server.
    @discussion Search base suffix for a LDAP server.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeLDAPSearchBaseSuffix;

/*!
    @const		kODAttributeTypeLocation
	@abstract   Represents the location a service is available from (usually domain name)
	@discussion Represents the location a service is available from (usually domain name).
				Typically found in service record types including kODRecordTypeAFPServer,
				kODRecordTypeLDAPServer, and kODRecordTypeWebServer.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeLocation;

/*!
    @const		kODAttributeTypeMapGUID
    @abstract   Represents the GUID for a record's map
    @discussion Represents the GUID for a record's map.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMapGUID;

/*!
    @const		kODAttributeTypeMCXFlags
    @abstract   Used by ManagedClient
    @discussion Used by ManagedClient.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMCXFlags;

/*!
    @const		kODAttributeTypeMCXSettings
    @abstract   Used by ManagedClient
    @discussion Used by ManagedClient.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMCXSettings;

/*!
    @const		kODAttributeTypeMailAttribute
    @abstract   Holds the mail account config data
    @discussion Holds the mail account config data.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMailAttribute;

/*!
    @const		kODAttributeTypeMetaAutomountMap
	@abstract   Used to query for kODRecordTypeAutomount entries
	@discussion Used to query for kODRecordTypeAutomount entries associated with a specific 
				kODRecordTypeAutomountMap.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMetaAutomountMap;

/*!
    @const		kODAttributeTypeMiddleName
    @abstract   Used for the middle name of user or person record.
    @discussion Used for the middle name of user or person record.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMiddleName;

/*!
    @const		kODAttributeTypeModificationTimestamp
	@abstract   Attribute showing date/time of record modification.
	@discussion Attribute showing date/time of record modification.
				Format is x.208 standard YYYYMMDDHHMMSSZ which is required as GMT time.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeModificationTimestamp;

/*!
    @const		kODAttributeTypeNFSHomeDirectory
    @abstract   Defines a user's home directory mount point on the local machine.
    @discussion Defines a user's home directory mount point on the local machine.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNFSHomeDirectory;

/*!
    @const		kODAttributeTypeNote
    @abstract   Note attribute.
    @discussion Note attribute.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNote;

/*!
    @const		kODAttributeTypeOperatingSystem
    @abstract   Returns the operating system type where the daemon is running
    @discussion Returns the operating system type where the daemon is running,
				e.g., Mac OS X or Mac OS X Server
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeOperatingSystem __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
	@const		kODAttributeTypeOperatingSystemVersion
	@abstract   Returns the operating system version where the daemon is running
	@discussion Returns the operating system version where the daemon is running,
				e.g., 10.6
 */
CF_EXPORT
const ODAttributeType kODAttributeTypeOperatingSystemVersion __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @const		kODAttributeTypeOwner
	@abstract   Attribute type for the owner of a record. 
	@discussion Attribute type for the owner of a record. 
				Typically the value is a LDAP distinguished name.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeOwner;

/*!
    @const		kODAttributeTypeOwnerGUID
	@abstract   Attribute type for the owner GUID of a group. 
	@discussion Attribute type for the owner GUID of a group. 
				Found in group records (kODRecordTypeGroups). 
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeOwnerGUID;

/*!
    @const		kODAttributeTypePassword
    @abstract   Holds the password or credential value.
    @discussion Holds the password or credential value.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePassword;

/*!
    @const		kODAttributeTypePasswordPlus
    @abstract   Holds marker data to indicate possible authentication redirection.
    @discussion Holds marker data to indicate possible authentication redirection.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePasswordPlus;

/*!
    @const		kODAttributeTypePasswordPolicyOptions
	@abstract   Collection of password policy options in single attribute.
	@discussion Collection of password policy options in single attribute.
				Used in user presets record.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePasswordPolicyOptions;

/*!
    @const		kODAttributeTypePasswordServerList
    @abstract   Represents the attribute for storing the password server's replication information.
    @discussion Represents the attribute for storing the password server's replication information.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePasswordServerList;

/*!
    @const		kODAttributeTypePasswordServerLocation
	@abstract   Specifies the IP address or domain name of the Password Server associated
				with a given directory node.
	@discussion Specifies the IP address or domain name of the Password Server associated
				with a given directory node. Found in a config record named PasswordServer.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePasswordServerLocation;

/*!
    @const		kODAttributeTypePicture
	@abstract   Represents the path of the picture for each user displayed in the login window.
	@discussion Represents the path of the picture for each user displayed in the login window.
				Found in user records (kODRecordTypeUsers).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePicture;

/*!
    @const		kODAttributeTypePort
	@abstract   Represents the port number a service is available on.
	@discussion Represents the port number a service is available on.
				Typically found in service record types including kODRecordTypeAFPServer,
				kODRecordTypeLDAPServer, and kODRecordTypeWebServer.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePort;

/*!
    @const		kODAttributeTypePresetUserIsAdmin
	@abstract   Flag to indicate whether users created from this preset are administrators
				by default.
	@discussion Flag to indicate whether users created from this preset are administrators
				by default. Found in kODRecordTypePresetUsers records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePresetUserIsAdmin;

/*!
    @const		kODAttributeTypePrimaryComputerGUID
	@abstract   An attribute that defines a primary computer of the computer group.  
	@discussion An attribute that defines a primary computer of the computer group.  
				Added to computer group record type (kODRecordTypeComputerGroups)
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrimaryComputerGUID;

/*!
    @const		kODAttributeTypePrimaryComputerList
    @abstract   The GUID of the computer list with which this computer record is associated.
    @discussion The GUID of the computer list with which this computer record is associated.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrimaryComputerList;

/*!
    @const		kODAttributeTypePrimaryGroupID
	@abstract   This is the 32 bit unique ID that represents the primary group 
				a user is part of, or the ID of a group.
	@discussion This is the 32 bit unique ID that represents the primary group 
				a user is part of, or the ID of a group. Format is a signed 32 bit integer
				represented as a string.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrimaryGroupID;

/*!
    @const		kODAttributeTypePrinter1284DeviceID
	@abstract   Attribute that defines the IEEE 1284 DeviceID of a printer.
	@discussion Attribute that defines the IEEE 1284 DeviceID of a printer.
				This is used when configuring a printer.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrinter1284DeviceID;

/*!
    @const		kODAttributeTypePrinterLPRHost
    @abstract   Standard attribute type for kODRecordTypePrinters.
    @discussion Standard attribute type for kODRecordTypePrinters.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrinterLPRHost;

/*!
    @const		kODAttributeTypePrinterLPRQueue
    @abstract   Standard attribute type for kODRecordTypePrinters.
    @discussion Standard attribute type for kODRecordTypePrinters.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrinterLPRQueue;

/*!
    @const		kODAttributeTypePrinterMakeAndModel
	@abstract   Attribute for definition of the Printer Make and Model.
	@discussion Attribute for definition of the Printer Make and Model.  An example
				value would be "HP LaserJet 2200".  This would be used to determine the proper PPD
				file to be used when configuring a printer from the Directory.  This attribute
				is based on the IPP Printing Specification RFC and IETF IPP-LDAP Printer Record.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrinterMakeAndModel;

/*!
    @const		kODAttributeTypePrinterType
    @abstract   Standard attribute type for kODRecordTypePrinters.
    @discussion Standard attribute type for kODRecordTypePrinters.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrinterType;

/*!
    @const		kODAttributeTypePrinterURI
	@abstract   Attribute that defines the URI of a printer "ipp://address" or
				"smb://server/queue".
	@discussion Attribute that defines the URI of a printer "ipp://address" or
				"smb://server/queue".  This is used when configuring a printer. This attribute
				is based on the IPP Printing Specification RFC and IETF IPP-LDAP Printer Record.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrinterURI;

/*!
    @const		kODAttributeTypePrinterXRISupported
	@abstract   Attribute that defines additional URIs supported by a printer.
	@discussion attribute that defines additional URIs supported by a printer.
				This is used when configuring a printer. This attribute is based on the IPP 
				Printing Specification RFC and IETF IPP-LDAP Printer Record.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrinterXRISupported;

/*!
    @const		kODAttributeTypePrintServiceInfoText
    @abstract   Standard attribute type for kODRecordTypePrinters.
    @discussion Standard attribute type for kODRecordTypePrinters.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrintServiceInfoText;

/*!
    @const		kODAttributeTypePrintServiceInfoXML
    @abstract   Standard attribute type for kODRecordTypePrinters.
    @discussion Standard attribute type for kODRecordTypePrinters.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrintServiceInfoXML;

/*!
    @const		kODAttributeTypePrintServiceUserData
	@abstract   Attribute for print quota configuration or statistics (XML data).
	@discussion Attribute for print quota configuration or statistics (XML data).
				Found in user records (kODRecordTypeUsers) or print service
				statistics records (kODRecordTypePrintServiceUser).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrintServiceUserData;

/*!
    @const		kODAttributeTypeRealUserID
    @abstract   Used by Managed Client.
    @discussion Used by Managed Client
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeRealUserID;

/*!
    @const		kODAttributeTypeRelativeDNPrefix
	@abstract   Used to map the first native LDAP attribute type required in the building of the
				Relative Distinguished Name for LDAP record creation.
	@discussion Used to map the first native LDAP attribute type required in the building of the
				Relative Distinguished Name for LDAP record creation.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeRelativeDNPrefix;

/*!
    @const		kODAttributeTypeSMBAcctFlags
    @abstract   Account control flag.
    @discussion Account control flag.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBAcctFlags;

/*!
    @const		kODAttributeTypeSMBGroupRID
    @abstract   Constant for supporting PDC SMB interaction with DirectoryService.
    @discussion Constant for supporting PDC SMB interaction with DirectoryService.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBGroupRID;

/*!
    @const		kODAttributeTypeSMBHome
    @abstract   UNC address of Windows homedirectory mount point (\\server\\sharepoint).
    @discussion UNC address of Windows homedirectory mount point (\\server\\sharepoint).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBHome;

/*!
    @const		kODAttributeTypeSMBHomeDrive
    @abstract   Drive letter for homedirectory mount point.
    @discussion Drive letter for homedirectory mount point.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBHomeDrive;

/*!
    @const		kODAttributeTypeSMBKickoffTime
    @abstract   Attribute in support of SMB interaction.
    @discussion Attribute in support of SMB interaction.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBKickoffTime;

/*!
    @const		kODAttributeTypeSMBLogoffTime
    @abstract   Attribute in support of SMB interaction.
    @discussion Attribute in support of SMB interaction.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBLogoffTime;

/*!
    @const		kODAttributeTypeSMBLogonTime
    @abstract   Attribute in support of SMB interaction.
    @discussion Attribute in support of SMB interaction.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBLogonTime;

/*!
    @const		kODAttributeTypeSMBPrimaryGroupSID
	@abstract   SMB Primary Group Security ID, stored as a string attribute of
				up to 64 bytes.
	@discussion SMB Primary Group Security ID, stored as a string attribute of
				up to 64 bytes. Found in user, group, and computer records
				(kODRecordTypeUsers, kODRecordTypeGroups, kODRecordTypeComputers).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBPrimaryGroupSID;

/*!
    @const		kODAttributeTypeSMBPWDLastSet
    @abstract   Attribute in support of SMB interaction.
    @discussion Attribute in support of SMB interaction.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBPWDLastSet;

/*!
    @const		kODAttributeTypeSMBProfilePath
    @abstract   Desktop management info (dock, desktop links, etc).
    @discussion Desktop management info (dock, desktop links, etc).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBProfilePath;

/*!
    @const		kODAttributeTypeSMBRID
    @abstract   Attribute in support of SMB interaction.
    @discussion Attribute in support of SMB interaction.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBRID;

/*!
    @const		kODAttributeTypeSMBScriptPath
    @abstract   Login script path.
    @discussion Login script path.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBScriptPath;

/*!
    @const		kODAttributeTypeSMBSID
	@abstract   SMB Security ID, stored as a string attribute of up to 64 bytes.
	@discussion SMB Security ID, stored as a string attribute of up to 64 bytes.
				Found in user, group, and computer records (kODRecordTypeUsers, 
				kODRecordTypeGroups, kODRecordTypeComputers).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBSID;

/*!
    @const		kODAttributeTypeSMBUserWorkstations
    @abstract   List of workstations user can login from (machine account names).
    @discussion List of workstations user can login from (machine account names).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSMBUserWorkstations;

/*!
    @const		kODAttributeTypeServiceType
	@abstract   Represents the service type for the service.  This is the raw service type of the
				service.
	@discussion Represents the service type for the service.  This is the raw service type of the
				service.  For example a service record type of kODRecordTypeWebServer 
				might have a service type of "http" or "https".
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeServiceType;

/*!
    @const		kODAttributeTypeSetupAdvertising
    @abstract   Used for Setup Assistant automatic population.
    @discussion Used for Setup Assistant automatic population.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSetupAdvertising;

/*!
    @const		kODAttributeTypeSetupAutoRegister
    @abstract   Used for Setup Assistant automatic population.
    @discussion Used for Setup Assistant automatic population.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSetupAutoRegister;

/*!
    @const		kODAttributeTypeSetupLocation
    @abstract   Used for Setup Assistant automatic population.
    @discussion Used for Setup Assistant automatic population.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSetupLocation;

/*!
    @const		kODAttributeTypeSetupOccupation
    @abstract   Used for Setup Assistant automatic population.
    @discussion Used for Setup Assistant automatic population.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSetupOccupation;

/*!
    @const		kODAttributeTypeTimeToLive
	@abstract   Attribute recommending how long to cache the record's attribute values.
	@discussion Attribute recommending how long to cache the record's attribute values.
				Format is an unsigned 32 bit representing seconds. ie. 300 is 5 minutes.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeTimeToLive;

/*!
	@const		kODAttributeTypeTrustInformation
	@abstract	Used to describe a node's trust information.
	@discussion	Used to describe a node's trust information.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeTrustInformation;

/*!
    @const		kODAttributeTypeUniqueID
	@abstract   This is the 32 bit unique ID that represents the user in the legacy manner.
	@discussion This is the 32 bit unique ID that represents the user in the legacy manner.
				Format is a signed integer represented as a string.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeUniqueID;

/*!
    @const		kODAttributeTypeUserCertificate
	@abstract   Attribute containing the binary of the user's certificate.
	@discussion Attribute containing the binary of the user's certificate.
				Usually found in user records. The certificate is data which identifies a user.
				This data is attested to by a known party, and can be independently verified 
				by a third party.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeUserCertificate;

/*!
    @const		kODAttributeTypeUserPKCS12Data
	@abstract   Attribute containing binary data in PKCS #12 format. 
	@discussion Attribute containing binary data in PKCS #12 format. 
				Usually found in user records. The value can contain keys, certificates,
				and other related information and is encrypted with a passphrase.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeUserPKCS12Data;

/*!
    @const		kODAttributeTypeUserShell
    @abstract   Used to represent the user's shell setting.
    @discussion Used to represent the user's shell setting.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeUserShell;

/*!
    @const		kODAttributeTypeUserSMIMECertificate
	@abstract   Attribute containing the binary of the user's SMIME certificate.
	@discussion Attribute containing the binary of the user's SMIME certificate.
				Usually found in user records. The certificate is data which identifies a user.
				This data is attested to by a known party, and can be independently verified 
				by a third party. SMIME certificates are often used for signed or encrypted
				emails.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeUserSMIMECertificate;

/*!
    @const		kODAttributeTypeVFSDumpFreq
    @abstract   Attribute used to support mount records.
    @discussion Attribute used to support mount records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeVFSDumpFreq;

/*!
    @const		kODAttributeTypeVFSLinkDir
    @abstract   Attribute used to support mount records.
    @discussion Attribute used to support mount records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeVFSLinkDir;

/*!
    @const		kODAttributeTypeVFSPassNo
    @abstract   Attribute used to support mount records.
    @discussion Attribute used to support mount records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeVFSPassNo;

/*!
    @const		kODAttributeTypeVFSType
    @abstract   Attribute used to support mount records.
    @discussion Attribute used to support mount records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeVFSType;

/*!
    @const		kODAttributeTypeWeblogURI
	@abstract   attribute that defines the URI of a user's weblog.
	@discussion attribute that defines the URI of a user's weblog.
				Usually found in user records (kODRecordTypeUsers). 
				Example: http://example.com/blog/jsmith
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeWeblogURI;

/*!
    @const		kODAttributeTypeXMLPlist
    @abstract   XML plist used.
    @discussion XML plist used.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeXMLPlist;

/*!
    @const		kODAttributeTypeProtocolNumber
	@abstract   Attribute that defines a protocol number.
	@discussion Attribute that defines a protocol number.  Usually found
				in protocol records (kODRecordTypeProtocols)
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeProtocolNumber;

/*!
    @const		kODAttributeTypeRPCNumber
	@abstract   Attribute that defines an RPC number.
	@discussion Attribute that defines an RPC number.  Usually found
				in RPC records (kODRecordTypeRPC)
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeRPCNumber;

/*!
    @const		kODAttributeTypeNetworkNumber
	@abstract   Attribute that defines a network number.
	@discussion Attribute that defines a network number.  Usually found
				in network records (kODRecordTypeNetworks)
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNetworkNumber;

/*!
    @const		kODAttributeTypeAccessControlEntry
    @abstract   Attribute type which stores directory access control directives.
    @discussion Attribute type which stores directory access control directives.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAccessControlEntry;

/*!
    @const		kODAttributeTypeAddressLine1
    @abstract   Line one of multiple lines of address data for a user.
    @discussion Line one of multiple lines of address data for a user.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAddressLine1;

/*!
    @const		kODAttributeTypeAddressLine2
    @abstract   Line two of multiple lines of address data for a user.
    @discussion Line two of multiple lines of address data for a user.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAddressLine2;

/*!
    @const		kODAttributeTypeAddressLine3
    @abstract   Line three of multiple lines of address data for a user.
    @discussion Line three of multiple lines of address data for a user.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAddressLine3;

/*!
    @const		kODAttributeTypeAreaCode
    @abstract   Area code of a user's phone number.
    @discussion Area code of a user's phone number.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAreaCode;

/*!
    @const		kODAttributeTypeAuthenticationAuthority
	@abstract   Determines what mechanism is used to verify or set a user's password.
	@discussion Determines what mechanism is used to verify or set a user's password.
				If multiple values are present, the first attributes returned take precedence.
				Typically found in User records (kODRecordTypeUsers).
 
				Authentication authorities are a multi-part string separated by semi-colons.
				One component is the "type" of authority, such as those listed below:
 
					"basic"					- is a crypt password
					"ShadowHash"			- is a hashed password stored in a secure location
					"ApplePasswordServer"	- is a password server-based account
					"Kerberosv5"			- is a Kerberosv5 based
					"LocalCachedUser"		- is a cached account based on an account from another node, using a ShadowHash password
					"DisabledUser"			- is an account that has been disabled
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAuthenticationAuthority;

/*!
    @const		kODAttributeTypeAutomountInformation
	@abstract   Determines what mechanism is used to verify or set a user's password.
	@discussion Determines what mechanism is used to verify or set a user's password.
				If multiple values are present, the first attributes returned take precedence.
				Typically found in User records (kODRecordTypeUsers).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAutomountInformation;

/*!
    @const		kODAttributeTypeBootParams
    @abstract   Attribute type in host or machine records for storing boot params.
    @discussion Attribute type in host or machine records for storing boot params.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeBootParams;

/*!
    @const		kODAttributeTypeBuilding
	@abstract   Represents the building name for a user or person record.
	@discussion Represents the building name for a user or person record.
				Usually found in user or people records (kODRecordTypeUsers or 
				kODRecordTypePeople).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeBuilding;

/*!
    @const		kODAttributeTypeServicesLocator
    @abstract   the URI for a record's calendar
    @discussion the URI for a record's calendar
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeServicesLocator;

/*!
    @const		kODAttributeTypeCity
	@abstract   Usually, city for a user or person record.
	@discussion Usually, city for a user or person record.
				Usually found in user or people records (kODRecordTypeUsers or 
				kODRecordTypePeople).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeCity;

/*!
    @const		kODAttributeTypeCompany
	@abstract   attribute that defines the user's company.
	@discussion attribute that defines the user's company.
				Example: Apple Inc.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeCompany;

/*!
    @const		kODAttributeTypeComputers
    @abstract   List of computers.
    @discussion List of computers.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeComputers;

/*!
    @const		kODAttributeTypeCountry
	@abstract   Represents country of a record entry.
	@discussion Represents country of a record entry.
				Usually found in user or people records (kODRecordTypeUsers or 
				kODRecordTypePeople).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeCountry;

/*!
    @const		kODAttributeTypeDepartment
	@abstract   Represents the department name of a user or person.
	@discussion Represents the department name of a user or person.
				Usually found in user or people records (kODRecordTypeUsers or 
				kODRecordTypePeople).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeDepartment;

/*!
    @const		kODAttributeTypeDNSName
    @abstract   DNS Resolver nameserver attribute.
    @discussion DNS Resolver nameserver attribute.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeDNSName;

/*!
    @const		kODAttributeTypeEMailAddress
    @abstract   Email address of usually a user record.
    @discussion Email address of usually a user record.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeEMailAddress;

/*!
    @const		kODAttributeTypeEMailContacts
	@abstract   Attribute that defines a record's custom email addresses.
	@discussion Attribute that defines a record's custom email addresses.
				found in user records (kODRecordTypeUsers). 
				Example: home:johndoe\@mymail.com
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeEMailContacts;

/*!
    @const		kODAttributeTypeFaxNumber
	@abstract   Represents the FAX numbers of a user or person.
	@discussion Represents the FAX numbers of a user or person.
				Usually found in user or people records (kODRecordTypeUsers or 
				kODRecordTypePeople).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeFaxNumber;

/*!
    @const		kODAttributeTypeGroup
    @abstract   List of groups.
    @discussion List of groups.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeGroup;

/*!
    @const		kODAttributeTypeGroupMembers
    @abstract   Attribute type in group records containing lists of GUID values for members other than groups.
    @discussion Attribute type in group records containing lists of GUID values for members other than groups.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeGroupMembers;

/*!
    @const		kODAttributeTypeGroupMembership
    @abstract   Usually a list of users that below to a given group record.
    @discussion Usually a list of users that below to a given group record.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeGroupMembership;

/*!
    @const		kODAttributeTypeGroupServices
	@abstract   xml-plist attribute that defines a group's services.
	@discussion xml-plist attribute that defines a group's services.
				Found in group records (kODRecordTypeGroups). 
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeGroupServices;

/*!
    @const		kODAttributeTypeHomePhoneNumber
    @abstract   Home telephone number of a user or person.
    @discussion Home telephone number of a user or person.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeHomePhoneNumber;

/*!
    @const		kODAttributeTypeHTML
    @abstract   HTML location.
    @discussion HTML location.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeHTML;

/*!
    @const		kODAttributeTypeHomeDirectory
    @abstract   Network home directory URL.
    @discussion Network home directory URL.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeHomeDirectory;

/*!
    @const		kODAttributeTypeIMHandle
	@abstract   Represents the Instant Messaging handles of a user.
	@discussion Represents the Instant Messaging handles of a user.
				Values should be prefixed with the appropriate IM type
				(i.e., AIM:, Jabber:, MSN:, Yahoo:, or ICQ:).
				Usually found in user records (kODRecordTypeUsers).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeIMHandle;

/*!
    @const		kODAttributeTypeIPAddress
    @abstract   IP address expressed either as domain or IP notation.
    @discussion IP address expressed either as domain or IP notation.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeIPAddress;

/*!
    @const		kODAttributeTypeIPAddressAndENetAddress
	@abstract   A pairing of IPv4 or IPv6 addresses with Ethernet addresses 
	@discussion A pairing of IPv4 or IPv6 addresses with Ethernet addresses 
				(e.g., "10.1.1.1/00:16:cb:92:56:41").  Usually found on kODRecordTypeComputers for use by 
				services that need specific pairing of the two values.  This should be in addition to 
				kODAttributeTypeIPAddress, kODAttributeTypeIPv6Address and kODAttributeTypeENetAddress. This is
				necessary because not all directories return attribute values in a guaranteed order.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeIPAddressAndENetAddress;

/*!
    @const		kODAttributeTypeIPv6Address
	@abstract   IPv6 address expressed in the standard notation
	@discussion IPv6 address expressed in the standard notation (e.g., "fe80::236:caff:fcc2:5641")
				Usually found on kODRecordTypeComputers and kODRecordTypeHosts.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeIPv6Address;

/*!
    @const		kODAttributeTypeJPEGPhoto
	@abstract   Used to store binary picture data in JPEG format. 
	@discussion Used to store binary picture data in JPEG format. 
				Usually found in user, people or group records (kODRecordTypeUsers, 
				kODRecordTypePeople, kODRecordTypeGroups).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeJPEGPhoto;

/*!
    @const		kODAttributeTypeJobTitle
	@abstract   Represents the job title of a user.
	@discussion Represents the job title of a user.
				Usually found in user or people records (kODRecordTypeUsers or 
				kODRecordTypePeople).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeJobTitle;

/*!
    @const		kODAttributeTypeKDCAuthKey
    @abstract   KDC master key RSA encrypted with realm public key.
    @discussion KDC master key RSA encrypted with realm public key.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeKDCAuthKey;

/*!
    @const		kODAttributeTypeKeywords
    @abstract   Keywords using for searching capability.
    @discussion Keywords using for searching capability.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeKeywords;

/*!
    @const		kODAttributeTypeLDAPReadReplicas
    @abstract   List of LDAP server URLs which can each be used to read directory data.
    @discussion List of LDAP server URLs which can each be used to read directory data.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeLDAPReadReplicas;

/*!
    @const		kODAttributeTypeLDAPWriteReplicas
    @abstract   List of LDAP server URLs which can each be used to write directory data.
    @discussion List of LDAP server URLs which can each be used to write directory data.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeLDAPWriteReplicas;

/*!
    @const		kODAttributeTypeMapCoordinates
	@abstract   attribute that defines coordinates for a user's location.
	@discussion attribute that defines coordinates for a user's location .
				found in user records (kODRecordTypeUsers) and resource records (kODRecordTypeResources).
				Example: 7.7,10.6
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMapCoordinates;

/*!
    @const		kODAttributeTypeMapURI
	@abstract   attribute that defines the URI of a user's location.
	@discussion attribute that defines the URI of a user's location.
				Usually found in user records (kODRecordTypeUsers). 
				Example: http://example.com/bldg1
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMapURI;

/*!
    @const		kODAttributeTypeMIME
    @abstract   Data contained in this attribute type is a fully qualified MIME Type. 
    @discussion Data contained in this attribute type is a fully qualified MIME Type. 
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMIME;

/*!
    @const		kODAttributeTypeMobileNumber
	@abstract   Represents the mobile numbers of a user or person.
	@discussion Represents the mobile numbers of a user or person.
				Usually found in user or people records (kODRecordTypeUsers or 
				kODRecordTypePeople).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMobileNumber;

/*!
    @const		kODAttributeTypeNestedGroups
    @abstract   Attribute type in group records for the list of GUID values for nested groups.
    @discussion Attribute type in group records for the list of GUID values for nested groups.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNestedGroups;

/*!
    @const		kODAttributeTypeNetGroups
	@abstract   Attribute type that indicates which netgroups its record is a member of.
	@discussion Attribute type that indicates which netgroups its record is a member of.
				Found in user and host records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNetGroups;

/*!
    @const		kODAttributeTypeNickName
	@abstract   Represents the nickname of a user or person.
	@discussion Represents the nickname of a user or person.
				Usually found in user or people records (kODRecordTypeUsers or 
				kODRecordTypePeople).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNickName;

/*!
    @const		kODAttributeTypeOrganizationInfo
    @abstract   Usually the organization info of a user.
    @discussion Usually the organization info of a user.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeOrganizationInfo;

/*!
    @const		kODAttributeTypeOrganizationName
    @abstract   Usually the organization of a user.
    @discussion Usually the organization of a user.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeOrganizationName;

/*!
    @const		kODAttributeTypePagerNumber
	@abstract   Represents the pager numbers of a user or person.
	@discussion Represents the pager numbers of a user or person.
				Usually found in user or people records (kODRecordTypeUsers or 
				kODRecordTypePeople).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePagerNumber;

/*!
    @const		kODAttributeTypePhoneContacts
	@abstract   attribute that defines a record's custom phone numbers.
	@discussion attribute that defines a record's custom phone numbers.
				found in user or people records.
					Example: home fax:408-555-4444
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePhoneContacts;

/*!
    @const		kODAttributeTypePhoneNumber
    @abstract   Telephone number of a user.
    @discussion Telephone number of a user.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePhoneNumber;

/*!
    @const		kODAttributeTypePGPPublicKey
    @abstract   Pretty Good Privacy public encryption key.
    @discussion Pretty Good Privacy public encryption key.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePGPPublicKey;

/*!
    @const		kODAttributeTypePostalAddress
    @abstract   The postal address usually excluding postal code.
    @discussion The postal address usually excluding postal code.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePostalAddress;

/*!
    @const		kODAttributeTypePostalAddressContacts
	@abstract   attribute that defines a record's alternate postal addresses.
	@discussion attribute that defines a record's alternate postal addresses.
				Found in user records (kODRecordTypeUsers) and resource records (kODRecordTypeResources).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePostalAddressContacts;

/*!
    @const		kODAttributeTypePostalCode
    @abstract   The postal code such as zip code in the USA.
    @discussion The postal code such as zip code in the USA.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePostalCode;

/*!
    @const		kODAttributeTypeNamePrefix
	@abstract   Represents the title prefix of a user or person.
	@discussion Represents the title prefix of a user or person.
				ie. Mr., Ms., Mrs., Dr., etc.
				Usually found in user or people records (kODRecordTypeUsers or 
				kODRecordTypePeople).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNamePrefix;

/*!
    @const      kODAttributeTypeProfiles
    @abstract   Profiles associated with the record.
 */
CF_EXPORT
const ODAttributeType kODAttributeTypeProfiles __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODAttributeTypeProfilesTimestamp
    @abstract   Timestamp for the profiles associated with the record.
 */
CF_EXPORT
const ODAttributeType kODAttributeTypeProfilesTimestamp __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const		kODAttributeTypeProtocols
    @abstract   List of protocols.
    @discussion List of protocols.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeProtocols;

/*!
    @const		kODAttributeTypeRecordName
    @abstract   List of names/keys for this record.
    @discussion List of names/keys for this record.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeRecordName;

/*!
    @const		kODAttributeTypeRelationships
	@abstract   attribute that defines the relationship to the record type.
	@discussion attribute that defines the relationship to the record type.
				found in user records (kODRecordTypeUsers). 
					Example: brother:John
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeRelationships;

/*!
    @const		kODAttributeTypeResourceInfo
    @abstract   attribute that defines a resource record's info.
    @discussion attribute that defines a resource record's info.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeResourceInfo;

/*!
    @const		kODAttributeTypeResourceType
	@abstract   Attribute type for the kind of resource. 
	@discussion Attribute type for the kind of resource. 
				found in resource records (kODRecordTypeResources). 
					Example: ConferenceRoom
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeResourceType;

/*!
    @const		kODAttributeTypeState
    @abstract   The state or province of a country.
    @discussion The state or province of a country.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeState;

/*!
    @const		kODAttributeTypeStreet
	@abstract   Represents the street address of a user or person.
	@discussion Represents the street address of a user or person.
				Usually found in user or people records (kODRecordTypeUsers or 
				kODRecordTypePeople).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeStreet;

/*!
    @const		kODAttributeTypeNameSuffix
	@abstract   Represents the name suffix of a user or person.
	@discussion Represents the name suffix of a user or person.
				i.e., Jr., Sr., etc.
				Usually found in user or people records (kODRecordTypeUsers or 
				kODRecordTypePeople).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNameSuffix;

/*!
    @const		kODAttributeTypeURL
    @abstract   List of URLs.
    @discussion List of URLs.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeURL;

/*!
    @const		kODAttributeTypeVFSOpts
    @abstract   Used in support of mount records.
    @discussion Used in support of mount records.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeVFSOpts;

/*!
    @const		kODAttributeTypeAlias
    @abstract   Alias attribute, contain pointer to another node/record/attribute.
    @discussion Alias attribute, contain pointer to another node/record/attribute.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAlias;

/*!
    @const		kODAttributeTypeAuthCredential
    @abstract   An authentication credential, to be used to authenticate to a Directory.
    @discussion An authentication credential, to be used to authenticate to a Directory.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAuthCredential;

/*!
    @const		kODAttributeTypeCopyTimestamp
    @abstract   Timestamp used in local account caching.
    @discussion Timestamp used in local account caching.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeCopyTimestamp;

/*!
    @const		kODAttributeTypeDateRecordCreated
    @abstract   Date of record creation.
    @discussion Date of record creation.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeDateRecordCreated;

/*!
    @const		kODAttributeTypeKerberosRealm
    @abstract   Supports Kerberized SMB Server services.
    @discussion Supports Kerberized SMB Server services.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeKerberosRealm;

/*!
    @const		kODAttributeTypeNTDomainComputerAccount
    @abstract   Supports Kerberized SMB Server services.
    @discussion Supports Kerberized SMB Server services.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNTDomainComputerAccount;

/*!
    @const		kODAttributeTypeOriginalHomeDirectory
    @abstract   Home directory URL used in local account caching.
    @discussion Home directory URL used in local account caching.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeOriginalHomeDirectory;

/*!
    @const		kODAttributeTypeOriginalNFSHomeDirectory
    @abstract   NFS home directory used in local account caching.
    @discussion NFS home directory used in local account caching.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeOriginalNFSHomeDirectory;

/*!
    @const		kODAttributeTypeOriginalNodeName
    @abstract   Nodename used in local account caching.
    @discussion Nodename used in local account caching.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeOriginalNodeName;

/*!
    @const		kODAttributeTypePrimaryNTDomain
    @abstract   Supports Kerberized SMB Server services.
    @discussion Supports Kerberized SMB Server services.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrimaryNTDomain;

/*!
    @const		kODAttributeTypePwdAgingPolicy
    @abstract   Contains the password aging policy data for an authentication capable record.
    @discussion Contains the password aging policy data for an authentication capable record.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePwdAgingPolicy;

/*!
    @const		kODAttributeTypeReadOnlyNode
	@abstract   Can be found using dsGetDirNodeInfo and will return one of
				ReadOnly, ReadWrite, or WriteOnly strings.
	@discussion Can be found using dsGetDirNodeInfo and will return one of
				ReadOnly, ReadWrite, or WriteOnly strings.
				Note that ReadWrite does not imply fully readable or writable
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeReadOnlyNode;

/*!
    @const		kODAttributeTypeTimePackage
    @abstract   Data of Create, Modify, Backup time in UTC.
    @discussion Data of Create, Modify, Backup time in UTC.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeTimePackage;

/*!
    @const		kODAttributeTypeTotalSize
    @abstract   checksum/meta data.
    @discussion checksum/meta data.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeTotalSize;

/*!
    @const		kODAttributeTypeAuthMethod
    @abstract   Authentication method for an authentication capable record.
    @discussion Authentication method for an authentication capable record.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAuthMethod;

/*!
    @const		kODAttributeTypeMetaNodeLocation
    @abstract   Meta attribute returning registered node name by directory node plugin.
    @discussion Meta attribute returning registered node name by directory node plugin.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeMetaNodeLocation;

/*!
    @const		kODAttributeTypeNodePath
	@abstract   Attribute type in Neighborhood records describing the DS Node to search while
				looking up aliases in this neighborhood.
	@discussion Attribute type in Neighborhood records describing the DS Node to search while
				looking up aliases in this neighborhood.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNodePath;

/*!
    @const		kODAttributeTypePlugInInfo
	@abstract   Information (version, signature, about, credits, etc.) about the plug-in
				that is actually servicing a particular directory node.
	@discussion Information (version, signature, about, credits, etc.) about the plug-in
				that is actually servicing a particular directory node.
				Has never been supported.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePlugInInfo;

/*!
    @const		kODAttributeTypeRecordType
    @abstract   Attribute for a Record or a Directory Node.
    @discussion Attribute for a Record or a Directory Node.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeRecordType;

/*!
    @const		kODAttributeTypeSchema
    @abstract   List of attribute types.
    @discussion List of attribute types.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSchema;

/*!
    @const		kODAttributeTypeSubNodes
	@abstract   Attribute of a node which lists the available subnodes
				of that node.
	@discussion Attribute of a node which lists the available subnodes
				of that node.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSubNodes;

/*!
    @const		kODAttributeTypeNetGroupTriplet
	@abstract   Attribute that defines the host, user and domain triplet combinations
				to support NetGroups.  Each attribute value is comma separated string to maintain the
				triplet (e.g., host,user,domain).
	@discussion Attribute that defines the host, user and domain triplet combinations
				to support NetGroups.  Each attribute value is comma separated string to maintain the
				triplet (e.g., host,user,domain).
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNetGroupTriplet;

/*!
    @const		kODAttributeTypeSearchPath
    @abstract   Search path used by the search node.
    @discussion Search path used by the search node.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSearchPath;

/*!
    @const		kODAttributeTypeSearchPolicy
    @abstract   Search policy for the search node.
    @discussion Search policy for the search node.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeSearchPolicy;

/*!
    @const		kODAttributeTypeAutomaticSearchPath
    @abstract   Automatic search path defined by the search node.
    @discussion Automatic search path defined by the search node.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAutomaticSearchPath;

/*!
    @const		kODAttributeTypeLocalOnlySearchPath
    @abstract   Local only search path defined by the search node.
    @discussion Local only search path defined by the search node.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeLocalOnlySearchPath;

/*!
    @const		kODAttributeTypeCustomSearchPath
    @abstract   Admin user configured custom search path defined by the search node.
    @discussion Admin user configured custom search path defined by the search node.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeCustomSearchPath;

/*!
    @const      kODAttributeTypeNodeOptions
    @abstract   Any extended options supported by the node during creation
    @discussion Any extended options supported by the node during creation
 */
CF_EXPORT
const ODAttributeType kODAttributeTypeNodeOptions __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
    @const      kODAttributeTypeNodeSASLRealm
    @abstract   Contains the SASL realm associated with this node (if any)
    @discussion Contains the SASL realm associated with this node (if any)
 */
CF_EXPORT
const ODAttributeType kODAttributeTypeNodeSASLRealm __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
	@const		kODAttributeTypeAdvertisedServices
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAdvertisedServices __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
	@const		kODAttributeTypeLocaleRelay
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeLocaleRelay __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
	@const		kODAttributeTypeLocaleSubnets
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeLocaleSubnets __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
	@const		kODAttributeTypeNetworkInterfaces
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNetworkInterfaces __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
	@const		kODAttributeTypeParentLocales
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeParentLocales __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
	@const		kODAttributeTypePrimaryLocale
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePrimaryLocale __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

#pragma mark Attribute Types (Configure Node)

/*!
    @const		kODAttributeTypeBuildVersion
    @abstract   Build version for reference.
    @discussion Build version for reference.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeBuildVersion;

/*!
    @const		kODAttributeTypeConfigAvail
    @abstract   Config avail tag.
    @discussion Config avail tag.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeConfigAvailable;

/*!
    @const		kODAttributeTypeConfigFile
    @abstract   Config file name.
    @discussion Config file name.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeConfigFile;

/*!
    @const		kODAttributeTypeCoreFWVersion
    @abstract   Core FW version for reference.
    @discussion Core FW version for reference.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeCoreFWVersion;

/*!
    @const		kODAttributeTypeFunctionalState
    @abstract   Functional state of plugin for example.
    @discussion Functional state of plugin for example.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeFunctionalState;

/*!
    @const		kODAttributeTypeFWVersion
    @abstract   Framework version for reference.
    @discussion Framework version for reference.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeFWVersion;

/*!
    @const		kODAttributeTypePluginIndex
    @abstract   Plugin index for reference.
    @discussion Plugin index for reference.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePluginIndex;

/*!
    @const		kODAttributeTypeNumTableList
    @abstract   Summary of the reference table entries presented as attribute values 
				from the Configure node.
    @discussion Summary of the reference table entries presented as attribute values
				from the Configure node.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNumTableList;

/*!
    @const		kODAttributeTypeVersion
    @abstract   Version label.
    @discussion Version label.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeVersion;

/*!
    @const		kODAttributeTypePIDValue
    @abstract   PID value.
    @discussion PID value.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypePIDValue;

/*!
    @const		kODAttributeTypeProcessName
    @abstract   Process Name.
    @discussion Process Name.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeProcessName;

/*!
    @const		kODAttributeTypeTotalRefCount
    @abstract   Total count of references for a process.
    @discussion Total count of references for a process.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeTotalRefCount;

/*!
    @const		kODAttributeTypeDirRefCount
    @abstract   Directory reference count for a process.
    @discussion Directory reference count for a process.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeDirRefCount;

/*!
    @const		kODAttributeTypeNodeRefCount
    @abstract   Node reference count for a process.
    @discussion Node reference count for a process.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNodeRefCount;

/*!
    @const		kODAttributeTypeRecRefCount
    @abstract   Record reference count for a process.
    @discussion Record reference count for a process.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeRecRefCount;

/*!
    @const		kODAttributeTypeAttrListRefCount
    @abstract   Attribute List reference count for a process.
    @discussion Attribute List reference count for a process.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAttrListRefCount;

/*!
    @const		kODAttributeTypeAttrListValueRefCount
    @abstract   Attr List Value reference count for a process.
    @discussion Attr List Value reference count for a process.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAttrListValueRefCount;

/*!
    @const		kODAttributeTypeDirRefs
    @abstract   All the directory references for a process.
    @discussion All the directory references for a process.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeDirRefs;

/*!
    @const		kODAttributeTypeNodeRefs
    @abstract   All the node references for a process.
    @discussion All the node references for a process.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeNodeRefs;

/*!
    @const		kODAttributeTypeRecRefs
    @abstract   All the record references for a process.
    @discussion All the record references for a process.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeRecRefs;

/*!
    @const		kODAttributeTypeAttrListRefs
    @abstract   All the attribute list references for a process.
    @discussion All the attribute list references for a process.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAttrListRefs;

/*!
    @const		kODAttributeTypeAttrListValueRefs
    @abstract   All the attribute list value references for a process.
    @discussion All the attribute list value references for a process.
*/
CF_EXPORT
const ODAttributeType kODAttributeTypeAttrListValueRefs;

#pragma mark Authentication Types

/*!
    @const		kODAuthenticationType2WayRandom
	@abstract   Two way random authentication method.
	@discussion Two way random authentication method. This method uses two passes to
				complete the authentication.

				First pass authentication array has items:
					user name in UTF8 encoding

				Second pass authentication array has items:
					8 byte DES digest,
					8 bytes of random
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationType2WayRandom;

/*!
    @const		kODAuthenticationType2WayRandomChangePasswd
	@abstract   Change the password for a user using the two-way random method.
	@discussion Change the password for a user using the two-way random method.
				Does not require prior authentication.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					old password encrypted with new (should be 8 chars),
					new password encrypted with old (should be 8 chars)
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationType2WayRandomChangePasswd;

/*!
    @const		kODAuthenticationTypeAPOP
	@abstract   APOP authentication method.
	@discussion APOP authentication method.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					server challenge in UTF8 encoding,
					client response in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeAPOP;

/*!
    @const		kODAuthenticationTypeCRAM_MD5
	@abstract   CRAM MD5 authentication method.
	@discussion CRAM MD5 authentication method.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					server challenge in UTF8 encoding,
					client response data
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeCRAM_MD5;

/*!
    @const		kODAuthenticationTypeChangePasswd
	@abstract   Change the password for a user.
	@discussion Change the password for a user. Does not require prior authentication.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					old password in UTF8 encoding,
					new password in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeChangePasswd;

/*!
    @const		kODAuthenticationTypeClearText
	@abstract   Clear text authentication method.
	@discussion Clear text authentication method.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					password in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeClearText;

/*!
    @const		kODAuthenticationTypeClearTextReadOnly
	@abstract   Clear text authentication method.
	@discussion Clear text authentication method.

                Authentication array has following items in order:
                    user name in UTF8 encoding,
                    password in UTF8 encoding
 */
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeClearTextReadOnly;

/*!
    @const		kODAuthenticationTypeCrypt
	@abstract   Use a crypt password stored in the user record if available to
				do the authentication.
	@discussion Use a crypt password stored in the user record if available to
				do the authentication.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					password in UTF8 encoding

				This method may not be supported by all plug-ins or for all users.
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeCrypt;

/*!
    @const		kODAuthenticationTypeDIGEST_MD5
	@abstract   Digest MD5 authentication method.
	@discussion Digest MD5 authentication method.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					server challenge in UTF8 encoding,
					client response data,
					HTTP method in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeDIGEST_MD5;

/*!
    @const		kODAuthenticationTypeDeleteUser
	@abstract   Used for Apple password server user deletion.
	@discussion Used for Apple password server user deletion.
				This authentication method is only implemented by the PasswordServer node.

				Authentication array has following items in order:
					Password Server ID in UTF8 encoding,
					authenticator password in UTF8 encoding,
					user's Password Server ID in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeDeleteUser;

/*!
    @const		kODAuthenticationTypeGetEffectivePolicy
	@abstract   Used to extract, from a password server, the actual policies that will be applied
				to a user; a combination of global and user policies.
	@discussion Used to extract, from a password server, the actual policies that will be applied
				to a user; a combination of global and user policies.

				Authentication array has following items in order:
					user name or Password Server ID in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeGetEffectivePolicy;

/*!
    @const		kODAuthenticationTypeGetGlobalPolicy
	@abstract   Used for extraction of global authentication policy.
	@discussion Used for extraction of global authentication policy. Authentication
				is not required to get policies. The authenticator name and password
				fields are optional.

				Authentication array has following items in order:
					user name in UTF8 encoding
					password in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeGetGlobalPolicy;

/*!
    @const		kODAuthenticationTypeGetKerberosPrincipal
	@abstract   Retrieves Kerberos Principal name.
	@discussion Retrieves Kerberos Principal name.

				Authentication array has following items in order:
					user name in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeGetKerberosPrincipal;

/*!
    @const		kODAuthenticationTypeGetPolicy
	@abstract   The plug-in should determine which specific authentication method to use.
	@discussion The plug-in should determine which specific authentication method to use.
				Authentication is not required to get policies. The authenticator name and password
				fields may be left blank by using a length of 1 and a zero-byte for the data.

				Authentication array has following items in order:
					authenticator's name or Password Server ID in UTF8 encoding,
					authenticator's password in UTF8 encoding
					account's name or Password Server ID

				The Password Server does not require authentication for this authentication method.
				The first two fields are to cover us for future policy changes and to keep the buffer
				format as standardized as possible.
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeGetPolicy;

/*!
    @const		kODAuthenticationTypeGetUserData
	@abstract   Used with Apple password server.
	@discussion Used with Apple password server. The password server maintains a space
				for a small amount of miscellaneous data.
				This authentication method is only implemented by the PasswordServer node.

				Authentication array has following items in order:
					authenticator's Password Server ID in UTF8 encoding,
					authenticator's password in UTF8 encoding
					Password Server ID in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeGetUserData;

/*!
    @const		kODAuthenticationTypeGetUserName
	@abstract   Used with Apple password server.
	@discussion Used with Apple password server. This name is the same as the primary
				short name for the user.
				This authentication method is only implemented by the PasswordServer node.

				Authentication array has following items in order:
					authenticator's Password Server ID in UTF8 encoding,
					authenticator's password in UTF8 encoding,
					user's Password Server ID in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeGetUserName;

/*!
    @const		kODAuthenticationTypeKerberosTickets
	@abstract   Provides write-access to LDAP with an existing Kerberos ticket.
	@discussion Provides write-access to LDAP with an existing Kerberos ticket

				Authentication array has following items in order:
					user name in UTF8 encoding,
					krb5_data containing a service ticket
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeKerberosTickets;

/*!
    @const		kODAuthenticationTypeMPPEMasterKeys
	@abstract   Generated 40-bit or 128-bit master keys from MS-CHAPv2 credentials (RFC 3079).
	@discussion Generated 40-bit or 128-bit master keys from MS-CHAPv2 credentials (RFC 3079).

				Authentication array has following items in order:
					user name in UTF8 encoding,
					MS-CHAPv2 digest (P24),
					key size, 8 or 16 (packed as a byte, not a string)
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeMPPEMasterKeys;

/*!
    @const		kODAuthenticationTypeMSCHAP2
	@abstract	MS-CHAP2 is a mutual authentication method.
	@discussion	MS-CHAP2 is a mutual authentication method. The plug-in will generate the data to
				send back to the client and put it in the continue items array.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					server challenge,
					peer challenge,
					client's digest,
					client's user name (the name used for MS-CHAPv2, usually the first short name)

				Continue items array contains:
					digest for the client's challenge
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeMSCHAP2;

/*!
    @const		kODAuthenticationTypeNTLMv2
	@abstract   Verifies an NTLMv2 challenge and response.
	@discussion Verifies an NTLMv2 challenge and response. The session keys
				(if any) must be retrieved separately with a trusted authentication.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					SMB server challenge,
					the client "blob" which includes 16 bytes of client digest prefixed
						to the the blob data,
					the user name used to calculate the digest in UTF8 encoding,
					the SMB domain in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeNTLMv2;

/*!
    @const		kODAuthenticationTypeNTLMv2WithSessionKey
	@abstract   An optimized method that checks the user's challenge and response
				and retrieves session keys in a single call.
	@discussion An optimized method that checks the user's challenge and response
				and retrieves session keys in a single call. If the NTLMv2 session key is
				supported, it is returned in the step buffer.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					SMB server challenge,
					the client "blob" which includes 16 bytes of client digest prefixed
						to the the blob data,
					the user name used to calculate the digest  in UTF8 encoding,
					the SMB domain in UTF8 encoding,
					user name in UTF8 encoding,
					authenticator password in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeNTLMv2WithSessionKey;

/*!
    @const		kODAuthenticationTypeNewUser
	@abstract	Create a new user record with the authentication authority.
	@discussion	Create a new user record with the authentication authority
				This authentication method is only implemented by the PasswordServer node.

				Authentication array has following items in order:
					authenticator's Password Server ID in UTF8 encoding,
					authenticator's password in UTF8 encoding,
					user's short-name,
					user's password
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeNewUser;

/*!
    @const		kODAuthenticationTypeNewUserWithPolicy
	@abstract	Create a new user record with the authentication authority and initial policy settings.
	@discussion	Create a new user record with the authentication authority and initial policy settings
				This authentication method is only implemented by the PasswordServer node.

				Authentication array has following items in order:
					authenticator's Password Server ID in UTF8 encoding,
					authenticator's password in UTF8 encoding,
					user's short-name,
					user's password,
					policy string in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeNewUserWithPolicy;

/*!
    @const		kODAuthenticationTypeNodeNativeClearTextOK
	@abstract   The plug-in should determine which specific authentication method to use.
	@discussion The plug-in should determine which specific authentication method to use.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					password in UTF8 encoding

				The plug-in may choose to use a cleartext authentication method if necessary.
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeNodeNativeClearTextOK;

/*!
    @const		kODAuthenticationTypeNodeNativeNoClearText
	@abstract   The plug-in should determine which specific authentication method to use.
	@discussion The plug-in should determine which specific authentication method to use.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					password in UTF8 encoding

				The plug-in must not use an authentication method that sends the password in cleartext.
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeNodeNativeNoClearText;

/*!
    @const		kODAuthenticationTypeReadSecureHash
	@abstract   Returns the SHA1 or Seeded SHA1 hash for a local user.
	@discussion Returns the SHA1 or Seeded SHA1 hash for a local user
				Only accessible by root processes. Only implemented by the local node.

				Authentication array has following items in order:
					user's name in UTF8 encoding

				Continue items array contains:
					value, either the old 20-byte SHA1 or the new seeded 24-byte SHA1.
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeReadSecureHash;

/*!
    @const		kODAuthenticationTypeSMBNTv2UserSessionKey
	@abstract   Generate the ntlm-v2 user session key.
	@discussion Generate the ntlm-v2 user session key. Requires prior authentication with a trusted
				authentication method.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					8 byte server challenge
					client response buffer
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSMBNTv2UserSessionKey;

/*!
    @const		kODAuthenticationTypeSMBWorkstationCredentialSessionKey
    @abstract   Generates an SMB workstation credential session key.
    @discussion Generates an SMB workstation credential session key.
 
				Authentication array has following items in order:
					user name in UTF8 encoding,
					8 byte server challenge + 8 byte client challenge
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSMBWorkstationCredentialSessionKey;

/*!
    @const		kODAuthenticationTypeSMB_LM_Key
	@abstract   SMB Lan Manager authentication method.
	@discussion SMB Lan Manager authentication method.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					8 byte server challenge,
					24 byte client response
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSMB_LM_Key;

/*!
    @const		kODAuthenticationTypeSMB_NT_Key
	@abstract   SMB NT authentication method.
	@discussion SMB NT authentication method.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					8 byte server challenge,
					24 byte client response
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSMB_NT_Key;

/*!
    @const		kODAuthenticationTypeSMB_NT_UserSessionKey
	@abstract   Used by SMB to get session keys.
	@discussion Used by SMB to get session keys
				This authentication method is only implemented by the PasswordServer node.

				Authentication array has following items in order:
					Password Server ID in UTF8 encoding

				Continue items array contains:
					MD4( ntHash )
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSMB_NT_UserSessionKey;

/*!
    @const		kODAuthenticationTypeSMB_NT_WithUserSessionKey
	@abstract   Used by SMB to authenticate and get session keys.
	@discussion Used by SMB to authenticate and get session keys

				Authentication array has following items in order:
					user name in UTF8 encoding,
					8 byte server challenge,
					24 byte client response,
					authenticator name in UTF8 encoding,
					authenticator password in UTF8 encoding

				Continue items array contains:
					MD4( ntHash )
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSMB_NT_WithUserSessionKey;

/*!
    @const		kODAuthenticationTypeSecureHash
	@abstract   Authentication specifically using the secure hash.
	@discussion Authentication specifically using the secure hash.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					salted SHA1 hash
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSecureHash __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_6, __MAC_10_7, __IPHONE_NA, __IPHONE_NA);

/*!
	@const		kODAuthenticationTypeSetCertificateHashAsCurrent
	@abstract	Set certificate using the authenticated user's credentials.
	@discussion	Set certificate using the authenticated user's credentials.

				Authentication array has the following items in order:
					user name in UTF8 encoding
					hashed certificate data (40 hex characters)
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSetCertificateHashAsCurrent __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
    @const		kODAuthenticationTypeSetGlobalPolicy
	@abstract   Used to set the global policy.
	@discussion Used to set the global policy.

				Authentication array has following items in order:
					user name or Password Server ID in UTF8 encoding,
					password in UTF8 encoding,
					policy string in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSetGlobalPolicy;

/*!
    @const		kODAuthenticationTypeSetLMHash
	@abstract   Set the LAN Manager hash for an account.
	@discussion Set the LAN Manager hash for an account. This method requires prior authentication.
				Setting the LM hash for an account instead of the plain text password can cause the Windows
				password to get out-of-sync with the password for other services. Therefore, this
				authentication method should only be used when there is no other choice.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					LAN Manager hash buffer
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSetLMHash;

/*!
    @const		kODAuthenticationTypeSetNTHash
	@abstract   Set the NT hash for a user.
	@discussion Set the NT hash for a user. This method requires prior authentication.
				Setting the NT hash for an account instead of the plain text password can cause the Windows
				password to get out-of-sync with the password for other services. Therefore, this
				authentication method should only be used when there is no other choice.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					NT hash buffer
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSetNTHash;

/*!
    @const		kODAuthenticationTypeSetPassword
	@abstract   Set password method.
	@discussion Set password method.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					new password in UTF8 encoding,
					authenticator's name in UTF8 encoding,
					authenticator's password in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSetPassword;

/*!
    @const		kODAuthenticationTypeSetPasswordAsCurrent
	@abstract   Set password using the current credentials.
	@discussion Set password using the current credentials.
 
				Authentication array has following items in order:
					user name in UTF8 encoding,
					new password in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSetPasswordAsCurrent;

/*!
    @const		kODAuthenticationTypeSetPolicy
	@abstract   The plug-in should determine which specific authentication method to use.
	@discussion The plug-in should determine which specific authentication method to use.

				Authentication array has following items in order:
					authenticator's name or Password Server ID in UTF8 encoding,
					authenticator's password in UTF8 encoding,
					name or Password Server ID of the target account in UTF8 encoding,
					policy data
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSetPolicy;

/*!
    @const		kODAuthenticationTypeSetPolicyAsCurrent
	@abstract   A set policy for the password server.
	@discussion A set policy for the password server.

				Authentication array has following items in order:
					user name or Password Server ID of the target account in UTF8 encoding,
					policy data
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSetPolicyAsCurrent;

/*!
    @const		kODAuthenticationTypeSetUserData
	@abstract   Used for Apple password server.
	@discussion Used for Apple password server.
				This authentication method is only implemented by the PasswordServer node.

				Authentication array has following items in order:
					authenticator's Password Server ID in UTF8 encoding,
					authenticator's password in UTF8 encoding,
					Password Server ID in UTF8 encoding,
					user data
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSetUserData;

/*!
    @const		kODAuthenticationTypeSetUserName
	@abstract   Used for Apple password server.
	@discussion Used for Apple password server.
				This authentication method is only implemented by the PasswordServer node.

				Authentication array has following items in order:
					authenticator's Password Server ID in UTF8 encoding,
					authenticator's password in UTF8 encoding,
					Password Server ID in UTF8 encoding,
					user's short name in UTF8 encoding
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSetUserName;

/*!
    @const		kODAuthenticationTypeSetWorkstationPassword
	@abstract   Supports PDC SMB interaction with DS.
	@discussion Supports PDC SMB interaction with DS.
 
				Authentication array has following items in order:
					workstation's Password Server ID in UTF8 encoding,
					NT hash
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeSetWorkstationPassword;

/*!
    @const		kODAuthenticationTypeWithAuthorizationRef
	@abstract	Allows access to local directories as root with a valid AuthorizationRef.
	@discussion	Allows access to local directories as root with a valid AuthorizationRef.

				Authentication array has following items in order:
					externalized AuthorizationRef
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeWithAuthorizationRef;

/*!
    @const		kODAuthenticationTypeWriteSecureHash
	@abstract	Supports ONLY a root process to be able to directly write the secure hash of a user record.
	@discussion	Supports ONLY a root process to be able to directly write the secure hash of a user record.

				Authentication array has following items in order:
					user name in UTF8 encoding,
					salted SHA1 hash
*/
CF_EXPORT
const ODAuthenticationType kODAuthenticationTypeWriteSecureHash;

#pragma mark Policy Types

/*!
    @const      kODPolicyTypePasswordCannotBeAccountName
    @abstract   Boolean signifying the password must not contain the name of the account.
    @discussion Boolean signifying the password must not contain the name of the account.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypePasswordCannotBeAccountName __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypePasswordChangeRequired
    @abstract   Boolean stating the password should be changed on next logon.
    @discussion Should be set to a CFBooleanRef/NSNumber accordingly to signify the password should be changed at next logon.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypePasswordChangeRequired __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypePasswordHistory
    @abstract   The number of previous passwords that will be remembered.
    @discussion Should be set with a CFNumber/NSNumber accordingly with a range of 1 to 15 passwords.  Remove this policy or
                set to 0 if no history is tracked.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypePasswordHistory __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypePasswordMinimumNumberOfCharacters
    @abstract   The minimum number of characters a password must contain.
    @discussion A CFNumber/NSNumber signifying the minimum number of characters a password must contain.
 */
CF_EXPORT
const ODPolicyType kODPolicyTypePasswordMinimumNumberOfCharacters __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypePasswordMaximumNumberOfCharacters
    @abstract   The maximum number of characters a password can contain.
    @discussion A CFNumber/NSNumber signifying the maximum number of characters a password can contain.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypePasswordMaximumNumberOfCharacters __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypePasswordMaximumAgeInMinutes
    @abstract   Number of minutes before password must be changed.
    @discussion The value for the policy should be a CFNumberRef/NSNumber accordingly.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypePasswordMaximumAgeInMinutes __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypePasswordRequiresAlpha
    @abstract   Sets the number of alpha characters that must present in the password.
    @discussion A CFNumber/NSNumber signifying the number of alpha characters [A-Z][a-z] that must be present in the password.
                Note, not all modules will support this capability.  If you exceed the module's capabilities, it will reset to
                the maximum supported by that module.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypePasswordRequiresAlpha __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypePasswordRequiresMixedCase
    @abstract   Boolean signifying if password requires mixed case characters both upper and lower characters.
    @discussion Boolean signifying if password requires mixed case characters both upper and lower characters.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypePasswordRequiresMixedCase __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypePasswordRequiresNumeric
    @abstract   Sets the number of numeric characters that must present in the password.
    @discussion A CFNumber/NSNumber signifying the number of numeric characters [0-9] that must be present in the password.
                Note, not all modules will support this capability.  If you exceed the module's capabilities, it will reset to
                the maximum supported by that module.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypePasswordRequiresNumeric __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypePasswordRequiresSymbol
    @abstract   Sets the number of symbol characters that must present in the password.
    @discussion A CFNumber/NSNumber signifying the number of symbol characters that must be present in the password.
                Note, not all modules will support this capability.  If you exceed the module's capabilities, it will reset to
                the maximum supported by that module.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypePasswordRequiresSymbol __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypePasswordSelfModification
    @abstract   Boolean stating if the account is allowed to change their own password.
    @discussion Should be set to a CFBooleanRef/NSNumber accordingly to signify the if the account holder is allowed to change
                their own password.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypePasswordSelfModification __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypeAccountExpiresOnDate
    @abstract   Date when an account expires and becomes deactivated.
    @discussion A CFDate/NSDate signifying when an account expires will be deactivated.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypeAccountExpiresOnDate __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypeAccountMaximumFailedLogins
    @abstract   Sets the number of maximum failed logins allowed for the account.
    @discussion A CFNumber/NSNumber signifying the number of times a bad password can be entered before the account is locked out.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypeAccountMaximumFailedLogins __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypeAccountMaximumMinutesUntilDisabled
    @abstract   Sets the number of maximum number of minutes before this account is disabled automatically from password set.
    @discussion A CFNumber/NSNumber signifying the number of minutes before the account should be auto-disabled.  Note,
                calculation of this is based on timestamp of the password, therefore also preventing the password from
                being changed is required.
 */
CF_EXPORT
const ODPolicyType kODPolicyTypeAccountMaximumMinutesUntilDisabled __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypePasswordMinutesUntilFailedLoginReset
    @abstract   Number of minutes before an account locked out by bad passwords is automatically re-activated.
    @discussion The value for the policy should be a CFNumberRef/NSNumber accordingly.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypeAccountMinutesUntilFailedLoginReset __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @const      kODPolicyTypeMaximumMinutesOfNonUse
    @abstract   Maximum number of minutes that an account has not been used before it is deactivated.
    @discussion A CFNumber/NSNumber signifying the number of minutes before an account has not been used before it is deactivated.
*/
CF_EXPORT
const ODPolicyType kODPolicyTypeAccountMaximumMinutesOfNonUse __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
   @enum        Expiration constants
   @abstract    Special values that may be returned by ODRecordSecondsUntil*Expires().
   @constant    kODExpirationTimeExpired indicates the password or authentication
                has expired.
   @constant    kODExpirationTimeNeverExpires indicates the password or
                authentication will never expire. 
*/
enum {
    kODExpirationTimeExpired      = 0LL,
    kODExpirationTimeNeverExpires = -1LL,
};

/*!
    @typedef    kODPolicyKeyType
    @abstract   Type for the keys in a policy dictionary.
    @discussion Type for the keys in a policy dictionary.  These are the
                expected keys in a policy.  Some keys are optional.  For more
                information see the the specific key.  Some keys are used in
                individual policies, others in a policy set.
*/
#ifdef __OBJC__
typedef NSString *ODPolicyKeyType;
#else
typedef CFStringRef ODPolicyKeyType;
#endif

/*!
    @const      kODPolicyKeyIdentifier
    @abstract   Key for the policy identifier in a policy dictionary.
    @discussion Key for the policy identifier in a policy dictionary.  Required
                key in a policy dictionary.  The value of this key is a string
                that uniquely identifies the policy.  It can be anything from a
                GUID to a string that describes the policy (e.g. "max num chars"). 
 */
CF_EXPORT
const ODPolicyKeyType kODPolicyKeyIdentifier __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyKeyParameters
    @abstract   Key for the policy parameters, if any, in a policy dictionary.
    @discussion Key for the policy parameters, if any, in a policy dictionary.
                Optional key in a policy dictionary.  The value of this key is a
                dictionary containing any parameters that are relevant to the
                policy.  Parameters may be used for information purposes or to
                provide additional data to be used in the policy format string.
 */
CF_EXPORT
const ODPolicyKeyType kODPolicyKeyParameters __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyKeyContent
    @abstract   Key for the policy format string in a policy dictionary.
    @discussion Key for the policy format string in a policy dictionary.
                Required key in a policy dictionary.  The value of this key is a
                string containing the policy itself, from which a predicate will
                be created.  The predicate will be applied during policy
                evaluation. 
 */
CF_EXPORT
const ODPolicyKeyType kODPolicyKeyContent __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyKeyContentDescription
    @abstract   Key for the policy content description.
    @discussion Key for the policy content description.  Used in either a policy
                dictionary or in kODPolicyKeyEvaluationDetails dictionary.  It
                is an optional key in either dictionary.

                When used in a policy dictionary, the value of this key is a
                dictionary containing key/value pairs consisting of locale
                identifiers and localized descriptions of the policy content.

                When used in kODPolicyKeyEvaluationDetails dictionary, the value
                is a string containing the localized description of the policy
                content.
 */
CF_EXPORT
const ODPolicyKeyType kODPolicyKeyContentDescription __OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_NA);

/*!
    @const      kODPolicyKeyEvaluationDetails
    @abstract   Key containing details of the policy evaluation results.
    @discussion Key containing details of the policy evaluation results.  This
                key may be used in the userInfo portion of a CFErrorRef/NSError.
                The value of this key is an array of dictionaries, with each
                dictionary containing the results of an individual policy
                evaluation.  The keys in the details dictionaries are:
                   kODPolicyKeyIdentifier
                   kODPolicyKeyContentDescription
                   kODPolicyKeyPolicySatisfied
*/
CF_EXPORT
const ODPolicyKeyType kODPolicyKeyEvaluationDetails __OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_NA);

/*!
    @const      kODPolicyKeyPolicySatisfied
    @abstract   Key denoting if a specific policy was satisfied during evaluation.
    @discussion Key denoting if a specific policy was satisfied during evaluation.
                This key is used in the dictionaries in kODPolicyKeyEvaluationDetails.
                The value of this key will be true if the specific policy was
                satisfied and false if not.
*/
CF_EXPORT
const ODPolicyKeyType kODPolicyKeyPolicySatisfied __OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_NA);

/*!
    @typedef    ODPolicyCategoryType
    @abstract   Type for the policy categories.
    @discussion Type for the policy categories.  Policy categories are also keys
                in a policy set dictionary, where the value of each category is
                an array of policy dictionaries.
*/
#ifdef __OBJC__
typedef NSString *ODPolicyCategoryType;
#else
typedef CFStringRef ODPolicyCategoryType;
#endif

/*!
    @const      kODPolicyCategoryAuthentication
    @abstract   Category for policies controlling when authentications are allowed.
    @discussion Category for policies controlling when authentications are allowed.  
                Policies in this category are evaluated when determining if an
                authentication should be allowed or when authentications will
                expire.  This constant is also used as a key in a policy set
                dictionary with a value containing an array of policy
                dictionaries. 
 */
CF_EXPORT
ODPolicyCategoryType kODPolicyCategoryAuthentication __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyCategoryPasswordContent
    @abstract   Category for policies controlling content of passwords.
    @discussion Category for policies controlling content of passwords.
                Policies in this category are evaluated when determining if a
                password contains the required content, which is typically done
                during password changes.  This constant is also used as a key in
                a policy set dictionary with a value containing an array of
                policy dictionaries.
 */
CF_EXPORT
ODPolicyCategoryType kODPolicyCategoryPasswordContent __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyCategoryPasswordChange
    @abstract   Category for policies controlling when password require changing.
    @discussion Category for policies controlling when password require changing.
 */
CF_EXPORT
ODPolicyCategoryType kODPolicyCategoryPasswordChange __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);


/*!
    @typedef    kODPolicyAttributeType
    @abstract   Type for attributes that may be used in policies.
    @discussion Type for attributes that may be used in policies.  Policies in
                this category will be evaluated when determining if an
                authentication should be allowed or when determine when a
                password will expire.  This constant is also used as a key in a
                policy set dictionary with a value containing an array of policy
                dictionaries. 
*/
#ifdef __OBJC__
typedef NSString *ODPolicyAttributeType;
#else
typedef CFStringRef ODPolicyAttributeType;
#endif

/*!
    @const      kODPolicyAttributeRecordName
    @abstract   Policy attribute for the record name.
    @discussion Policy attribute for the record name.  May be used in policies
                to compare against other record attributes.  For example, this
                Password Content policy string prevents the password from being
                the same as the record name:
                    [NSString stringWithFormat:@"%@ != %@", kODPolicyAttributeRecordName, kODPolicyAttributePassword];
 */
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeRecordName __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeRecordType
    @abstract   Policy attribute for the record type.
    @discussion Policy attribute for the record type.  Could be used in policies
                to tailor behavior for a particular record type.  For example,
                this Password Content policy string would require computer
                passwords to be a minimum of 24 characters long:
                    [NSString stringWithFormat:@"%@ == %@ and %@ matches '.{24,}+'",
                              kODPolicyAttributeRecordType, kODRecordTypeComputer,
                              kODPolicyAttributePassword]; 
 */
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeRecordType __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);


/*!
    @const      kODPolicyAttributePassword
    @abstract   Policy attribute for the password.
    @discussion Policy attribute for the password in plain text.  May be used in
		policies to compare against other attributes or for evaluation
		against regular expressions.  Primarily useful in the Password
		Content policies.  For example, this policy string checks
		whether the password length is at least 8 characters: 
                    [NSString stringWithFormat:@"%@ matches '.{8,}+'", kODPolicyAttributePassword];
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributePassword __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributePasswordHashes
    @abstract   Policy attribute for the password hashes
    @discussion Policy attribute for the password hashes.  The value should be a
		CFArray containing one or more CFData values representing a
		password hash.

		Used in Password Content policy strings to compare the new
		password against the password history, for example:
                    [NSString stringWithFormat:@"none %@ in %@", kODPolicyAttributePasswordHashes, kODPolicyAttributePasswordHistory];
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributePasswordHashes __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributePasswordHistory
    @abstract   Policy attribute for the list of previous password hashes
    @discussion Policy attribute for the list of previous password hashes.  The
		value of this attribute is a CFArray containing one or more
		CFData elements representing a hash of a previous password.
		Hash types may be mixed.

		Used in Password Content policy strings to compare the new
		password against the password history, for example:
                    [NSString stringWithFormat:@"none %@ in %@", kODPolicyAttributePasswordHashes, kODPolicyAttributePasswordHistory];
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributePasswordHistory __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributePasswordHistoryDepth
    @abstract   Policy attribute for the number of previous hashed passwords to keep.
    @discussion Policy attribute for the number of previous hashed passwords to
                keep.  Should used as a key in the policy parameter dictionary,
                with a CFNumber value, specifying the number of password to keep.
                Should not be used in a policy string. 
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributePasswordHistoryDepth __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeCurrentDate
    @abstract   Policy attribute for the current date and time as a CFDate.
    @discussion Policy attribute for the current date and time as a CFDate used
                in policy strings to compare the current date against another
                date.  The following policy string would allow authentications
                until the expiration date: 
                    [NSString stringWithFormat:@"%@ < %@", kODPolicyAttributeCurrentDate, kODPolicyAttributeExpiresOnDate];

                Do not use kODPolicyAttributeCurrentDate in policies where date
                arithmetic is needed; for those policies use
                kODPolicyAttributeCurrentTime instead.
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeCurrentDate __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeCurrentTime
    @abstract   Policy attribute for the current date and time in seconds.
    @discussion Policy attribute for the current date and time in seconds since
                the Unix epoch.  Used in policy strings to compare the
                current time against other times.  Suitable for use in policies
                where "date arithmetic" is needed (i.e. adding/subtracting
                values to/from the current time or another time in seconds).
                Ensure all times and date arithmetic in the policy are specified
                in seconds.

                Note that kODPolicyAttributeExpiresEveryNDays needs to be
                converted to seconds to match the units of the other times.  The
                special keyword DAYS_TO_SECONDS can be used to accomplish this.

                In the policy below, password changes are required every 90 days
                (kODPolicyAttributeExpiresEveryNDays = 90).

                    [NSString stringWithFormat:@"%@ < %@ + (%@ * DAYS_TO_SECONDS)",
                              kODPolicyAttributeCurrentTime,
                              kODPolicyAttributeLastPasswordChangeTime,
                              kODPolicyAttributeExpiresEveryNDays];
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeCurrentTime __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeCurrentTimeOfDay
    @abstract   Policy attribute for the current time of day as a CFNumber.
    @discussion Policy attribute for the current time of day as a CFNumber, in
                24 hour time, i.e. the range is 0000 through 2359.  Does not
                contain any date information.

                This attribute is used in policies to compare the current time
                of day against another time of day.  For example, to allow
                authentications between the hours of 8:00 AM and 5:00 PM, the
                policy string would be (kODPolicyAttributeEnableAtTimeOfDay is
                set to 0800 and  kODPolicyAttributeExpiresAtTimeOfDay is 1700):
                    [NSString stringWithFormat:@"%@ > %@ and %@ < %@",
                              kODPolicyAttributeCurrentTimeOfDay,
                              kODPolicyAttributeEnableAtTimeOfDay,
                              kODPolicyAttributeCurrentTimeOfDay,
                              kODPolicyAttributeExpiresAtTimeOfDay];
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeCurrentTimeOfDay __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeCurrentDayOfWeek
    @abstract   Policy attribute for the current day of the week, as a CFNumber.
    @discussion Policy attribute for the current day of the week, as a CFNumber.  
                Specified in units appropriate for the local calendar.  The
                range is 1 through 7, with 1 representing the first day of the
                week in the local calendar, and 7 representing the last day of
                the week.

                This attribute is used in policies to compare the current day of
                the week against another day of the week.  For example, to
                enable authentications on Monday through Friday, the policy
                would be (kODPolicyAttributeEnableOnDayOfWeek is set to the
                number for Monday and kODPolicyAttributeExpiresOnDayOfWeek is
                set to the number for Friday):
                    [NSString stringWithFormat:@"%@ > %@ and %@ < %@",
                              kODPolicyAttributeCurrentDayOfWeek,
                              kODPolicyAttributeEnableOnDayOfWeek,
                              kODPolicyAttributeCurrentDayOfWeek,
                              kODPolicyAttributeExpiresOnDayOfWeek];
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeCurrentDayOfWeek __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeFailedAuthentications
    @abstract   Policy attribute for the number of failed authentications.
    @discussion Policy attribute for the number of failed authentications for
                the record.  Used in policies to compare against the maximum
                failed authentications.  The following policy would deny further
                authentications after 3 failed attempts (assumes
                kODPolicyAttributeMaximumFailedAuthentications is 3):
                    [NSString stringWithFormat:@"%@ < %@",
                              kODPolicyAttributeFailedAuthentications,
                              kODPolicyAttributeMaximumFailedAuthentications];
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeFailedAuthentications __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeMaximumFailedAuthentications
    @abstract   Policy attribute for the maximum failed authentication attempts.
    @discussion Policy attribute for the maximum failed authentication attempts.  
                Used as a key in policy parameter dictionary to specify the
                maximum allowable failed authentication attempts with a CFNumber
                value.  Also used in the policy string to compare against the
                number of failed authentication attempts.  This policy would
                disallow authentications after 3 failed attempts:
                    @{ kODPolicyKeyIdentifier  : @"maximum failed authentications",
                       kODPolicyKeyParameters  : @{ kODPolicyAttributeMaximumFailedAuthentications : @3 },
                       kODPolicyKeyContent     : [NSString stringWithFormat:@"%@ < %@",
                                                           kODPolicyAttributeFailedAuthentications,
                                                           kODPolicyAttributeMaximumFailedAuthentications] };
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeMaximumFailedAuthentications __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeLastFailedAuthenticationTime
    @abstract   Policy attribute for the time of the last failed auth attempt.
    @discussion Policy attribute for the time of the last failed auth attempt.
                CFNumber with a value representing the the number of seconds
                since the Unix epoch.  Used in policies to compare against other
                times.  As an example, this policy string would prevent another
                authentication within 10 seconds of a failed authentication:  
                    [NSString stringWithFormat:@"%@ > %@ + 10",
                              kODPolicyAttributeCurrentTime,
                              kODPolicyAttributeLastFailedAuthenticationTime];
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeLastFailedAuthenticationTime __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeLastAuthenticationTime
    @abstract   Policy attribute for the time of the last successful auth attempt.
    @discussion Policy attribute for the time of the last successful auth attempt.
                CFNumber with a value representing the number of seconds since
                the Unix epoch.  Used in policies to compare against another
                time.  

                The sample policy string below would deny authentications if
                there have been no authentications in the last 90 days.

                Note that the number of days needs to be converted to seconds to
                match the units of kODPolicyAttributeCurrentTime.  The special
                keyword DAYS_TO_SECONDS can be used for the conversion.

                    [NSString stringWithFormat:@"%@ < %@ + 90 * DAYS_TO_SECONDS",
                              kODPolicyAttributeCurrentTime,
                              kODPolicyAttributeLastAuthenticationTime];
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeLastAuthenticationTime __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeLastPasswordChangeTime
    @abstract   Policy attribute for time of the last password change.
    @discussion Policy attribute for time of the last password change.  The time
                is specified as the number of seconds since the Unix epoch.
                Used in policies to compare against other times.  Typically
                would be used in Password Change policies to expire a password
                at a certain time or interval.

                The policy string below requires a password change every 90 days
                (kODPolicyAttributeExpiresEveryNDays is set to 90).

                Note that kODPolicyAttributeExpiresEveryNDays needs to be
                converted to seconds to match the units of the other times.  The
                special keyword DAYS_TO_SECONDS can be used for the conversion.

                    [NSString stringWithFormat:@"%@ < %@ + %@ * DAYS_TO_SECONDS",
                              kODPolicyAttributeCurrentTime,
                              kODPolicyAttributeLastPasswordChangeTime,
                              kODPolicyAttributeExpiresEveryNDays];
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeLastPasswordChangeTime __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeNewPasswordRequiredTime
    @abstract   Policy attribute for the time when "new password required" was set.
    @discussion Policy attribute for the time when "new password required" was set.
                The time is specified as the number of seconds since the
                Unix epoch.

                This attribute may be used in the policy string.  Whenever it's
                used in the policy string, it must also be specified in the
                policy's parameter dictionary.

                This attribute is used to capture the time when the system
                administrator wants to force all users to change their
                passwords.  This would be a "one-time" change, i.e. once the
                user changed the password, the policy would apply.  The policy
                would have to be updated with a new time for
                kODPolicyAttributeNewPasswordRequiredTime in order to force a
                new round of password changes.

                For example:
                    @{ kODPolicyKeyIdentifier : @"change on next auth",
                       kODPolicyKeyParameters : @{ kODPolicyAttributeNewPasswordRequiredTime : @(<time>) },
                       kODPolicyKeyContent    : [NSString stringWithFormat:@"%@ < %@",
                                                          kODPolicyAttributeLastPasswordChangeTime,
                                                          kODPolicyAttributeNewPasswordRequiredTime]};
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeNewPasswordRequiredTime __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeCreationTime
    @abstract   Policy attribute for the record creation time.
    @discussion Policy attribute for the record creation time.  The time is
                specified as the number of seconds since the "reference date".
                Could be used to disable "temporary" accounts after a specific
                period of time.  

                The example below disables authentications after 10
                days after the account was created.

                Note that kODPolicyAttributeDaysUntilExpiration must be
                converted to seconds to match the units of the other times.  The
                special keyword DAYS_TO_SECONDS can be used for the conversion.

                    @{ kODPolicyKeyIdentifier : @"expires after 10 days",
                       kODPolicyKeyParameters : @{kODPolicyAttributeDaysUntilExpiration : @10 },
                       kODPolicyKeyContent    : [NSString stringWithFormat:@"%@ < %@ + (%@ * DAYS_TO_SECONDS)",
                                                          kODPolicyAttributeCurrentTime,
                                                          kODPolicyAttributeCreationTime,
                                                          kODPolicyAttributeDaysUntilExpiration]};
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeCreationTime __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeExpiresEveryNDays
    @abstract   Policy attribute for expires every N days.
    @discussion Policy attribute for expires every N days.  This attribute is
                used as a key in the policy parameter dictionary, with CFNumber
                value.  Also used in the policy strings. Typically would be used
                in Password Change policies to expire a password at a certain
                time or interval. 

                The example policy below would require a password change every
                90 days.

                Note that kODPolicyAttributeExpiresEveryNDays needs to be
                converted to seconds to match the units of the other times used
                in the policy.  The special keyword DAYS_TO_SECONDS can be used
                for the conversion.

                    @{ kODPolicyKeyIdentifier : @"expires every 90 days",
                       kODPolicyKeyParameters : @{ kODPolicyAttributeExpiresEveryNDays : @90 },
                       kODPolicyKeyContent    : [NSString stringWithFormat:@"%@ < %@ + %@ * DAYS_TO_SECONDS",
                                                          kODPolicyAttributeCurrentTime,
                                                          kODPolicyAttributeLastPasswordChangeTime,
                                                          kODPolicyAttributeExpiresEveryNDays]};
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeExpiresEveryNDays __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeEnableOnDate
    @abstract   Policy attribute for the "enable on" date.
    @discussion Policy attribute for the "enable on" date.  Used as a key in the
                policy parameter dictionary, with a CFDate value.   Also used in
                policy strings. The date is specified as a CFDate representing a
                fixed date, appropriate for the locale.  Use in policies when
                comparing other date-based attributes.

                This attribute is typically used Authentication policies to
                control when authentications are allowed. This policy would
                enable authentications on Jan 1, 2014 (assumes the date
                formatter is properly configured for the locale): 
                    @{ kODPolicyKeyIdentifier : @"enable on Jan 1",
                       kODPolicyKeyParameters : @{ kODPolicyAttributeEnableOnDate : [localFormatter dateWithString:@"01/01/2014"] },
                       kODPolicyKeyContent    : [NSString stringWithFormat:@"%@ >= %@",
                                                          kODPolicyAttributeCurrentDate,
                                                          kODPolicyAttributeEnableOnDate]};
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeEnableOnDate __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeExpiresOnDate
    @abstract   Policy attribute for the "expires on" date.
    @discussion Policy attribute for the "expires on" date.  Used as a key in the
                policy parameter dictionary, with a CFDate value.   Also used in
                policy strings. The date is specified as a CFDate representing a
                fixed date, appropriate for the locale.  Use in policies when
                comparing other date-based attributes.

                This attribute is typically used in Authentication policies to
                control when authentications are allowed. This policy would
                disallow authentications on Jan 1, 2014 (assumes the date
                formatter is properly configured for the locale): 
                    @{ kODPolicyKeyIdentifier : @"expires on Jan 1",
                       kODPolicyKeyParameters : @{ kODPolicyAttributeExpiresOnDate : [localFormatter dateWithString:@"01/01/2014"] },
                       kODPolicyKeyContent    : [NSString stringWithFormat:@"%@ < %@",
                                                          kODPolicyAttributeCurrentDate,
                                                          kODPolicyAttributeExpiresOnDate]};
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeExpiresOnDate __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeEnableOnDayOfWeek
    @abstract   Policy attribute for enable on a day of the week.
    @discussion Policy attribute for enable on a day of the week.  Specified as a
                in units appropriate for the local calendar.  The range is 1
                through 7, with 1 representing the first day of the week in the
                local calendar, and 7 representing the last day of the week. 

                This attribute is used as a key in the policy parameter
                dictionary, with a CFNumber value, and in policy strings.
                Typically used i policy strings to compare against the another
                day of the week.  For example, to allow authentications only on
                Monday through Friday, the policy would be (assumes Monday = 2
                and Friday = 6 in the local calendar): 
                    @{ kODPolicyKeyIdentifier : @"mon-fri only",
                       kODPolicyKeyParameters : @{ kODPolicyAttributeEnableOnDayOfWeek : @2,
                                                   kODPolicyAttributeExpiresOnDayOfWeek : @6 },
                       kOPolicyKeyPolicy      : [NSString stringWithFormat:@"%@ > %@ and %@ < %@",
                                                          kODPolicyAttributeCurrentDayOfWeek,
                                                          kODPolicyAttributeEnableOnDayOfWeek,
                                                          kODPolicyAttributeCurrentDayOfWeek,
                                                          kODPolicyAttributeExpiresOnDayOfWeek]};
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeEnableOnDayOfWeek __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeExpiresOnDayOfWeek
    @abstract   Policy attribute for expires on a day of the week.
    @discussion Policy attribute for expires on a day of the week.  Specified as a
                in units appropriate for the local calendar.  The range is 1
                through 7, with 1 representing the first day of the week in the
                local calendar, and 7 representing the last day of the week. 

                This attribute is used as a key in the policy parameter
                dictionary, with a CFNumber value, and in policy strings.
                Typically used i policy strings to compare against the another
                day of the week.  See the example above for kODPolicyAttributeEnableOnDayOfWeek.
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeExpiresOnDayOfWeek __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeEnableAtTimeOfDay
    @abstract   Policy attribute for enable at a specific time of day.
    @discussion Policy attribute for enable at a specific time of day.
                The time is specified in 24-hour time, with a range of 0000
                through 2359.  This attribute can be used as a key in the policy
                parameter dictionary, with a CFNumber value, and in the policy
                strings. 

                Typically used in authentication policies to control specific
                times of when when authentications are allowed.  For example, to
                enable authentications between the hours of 8:00 AM and 5:00 PM,
                the policy would be: 
                    @{ kODPolicyKeyIdentifier : @"school hours",
                       kODPolicyKeyParameters : @{ kODPolicyAttributeEnableAtTimeOfDay  : @800,
                                                   kODPolicyAttributeExpiresAtTimeOfDay : @1700 },
                       kODPolicyKeyContent    : [NSString stringWithFormat:@"%@ > %@ and %@ < %@",
                                                          kODPolicyAttributeCurrentTimeOfDay,
                                                          kODPolicyAttributeEnableAtTimeOfDay,
                                                          kODPolicyAttributeCurrentTimeOfDay,
                                                          kODPolicyAttributeExpiresAtTimeOfDay]};
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeEnableAtTimeOfDay __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @const      kODPolicyAttributeExpiresAtTimeOfDay
    @abstract   Policy attribute for expires at a specific time of day.
    @discussion Policy attribute for expires at a specific time of day.
                The time is specified in 24-hour time, with a range of 0000
                through 2359.  This attribute can be used as a key in the policy
                parameter dictionary, with a CFNumber value, and in the policy
                strings. 

                Typically used in authentication policies to control specific
                times of when when authentications are allowed.  See the example
                above for kODPolicyAttributeEnableAtTimeOfDay.
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeExpiresAtTimeOfDay __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);


/*!
    @const      kODPolicyAttributeDaysUntilExpiration
    @abstract   Policy attribute for the number of days until expiration.
    @discussion Policy attribute for the number of days until expiration.
                This attribute can be used as a key in the policy parameter
                dictionary with a CFNumber value representing some number of
                days.  It can also be used in the policy strings. 

                May be used in Authentication policies to expire authentications
                after some number of days or in Password Change policies to
                expire passwords.

                The example below disables authentications after 10
                days after the account was created.

                Note that kODPolicyAttributeDaysUntilExpiration must be
                converted to seconds to match the units of the other times.  The
                special keyword DAYS_TO_SECONDS can be used for the conversion.

                    @{ kODPolicyKeyIdentifier : @"expires after 10 days",
                       kODPolicyKeyParameters : @{kODPolicyAttributeDaysUntilExpiration : @10 },
                       kODPolicyKeyContent    : [NSString stringWithFormat:@"%@ < %@ + (%@ * DAYS_TO_SECONDS)",
                                                          kODPolicyAttributeCurrentTime,
                                                          kODPolicyAttributeCreationTime,
                                                          kODPolicyAttributeDaysUntilExpiration]};
*/
CF_EXPORT
ODPolicyAttributeType kODPolicyAttributeDaysUntilExpiration __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);


#pragma mark Errors

/*!
 @enum   	ODFrameworkErrors
 @abstract   Errors specific to the framework and to underlying calls
 @discussion Errors specific to the framework and to underlying calls
 @constant kODErrorSuccess is when operation was successful (if appropriate)
 @constant kODErrorSessionLocalOnlyDaemonInUse is when a Local Only session was initiated and is still active
 @constant kODErrorSessionNormalDaemonInUse is when the Normal daemon is still in use but request was issued for Local only
 @constant kODErrorSessionDaemonNotRunning is when the daemon is not running
 @constant kODErrorSessionDaemonRefused is when the daemon refused the session
 @constant kODErrorSessionProxyCommunicationError is when there was a communication error with the remote daemon
 @constant kODErrorSessionProxyVersionMismatch is when versions mismatch between the remote daemon and local framework
 @constant kODErrorSessionProxyIPUnreachable is when the provided kODSessionProxyAddress did not respond
 @constant kODErrorSessionProxyUnknownHost is when the provided kODSessionProxyAddress cannot be resolved
 @constant kODErrorNodeUnknownName is when the node name provided does not exist and cannot be opened
 @constant kODErrorNodeUnknownType is when the node type provided is not a known value
 @constant kODErrorNodeConnectionFailed is when a node connection failed (commonly server closed connection, etc.)
 @constant kODErrorNodeUnknownHost is when an invalid host is provided
 @constant kODErrorQuerySynchronize is an error code that is returned when a synchronize has been initiated
 @constant kODErrorQueryInvalidMatchType is when an invalid match type is provided in a query
 @constant kODErrorQueryUnsupportedMatchType is when plugin does not support the requirested match type
 @constant kODErrorQueryTimeout is when a query timed out during request
 @constant kODErrorRecordReadOnlyNode is when a record cannot be modified
 @constant kODErrorRecordPermissionError is when the changes requested were denied due to insufficient permissions
 @constant kODErrorRecordParameterError is when an invalid parameter was provided
 @constant kODErrorRecordInvalidType is when an invalid record type was provided
 @constant kODErrorRecordAlreadyExists is when a record create failed because the record already exists
 @constant kODErrorRecordTypeDisabled is when a particular record type is disabled by policy for a plugin
 @constant kODErrorRecordAttributeUnknownType is when an unknown attribute type is provided
 @constant kODErrorRecordAttributeNotFound is when an attribute requested is not found in the record
 @constant kODErrorRecordAttributeValueSchemaError is when an attribute value does not meet schema requirements
 @constant kODErrorRecordAttributeValueNotFound is when an attribute value is not found in a record
 @constant kODErrorCredentialsInvalid is when credentials provided are invalid with the current node
 @constant kODErrorCredentialsMethodNotSupported is when a particular extended method is not supported by the node
 @constant kODErrorCredentialsNotAuthorized is when an operation such as changing a password is not authorized with current privileges
 @constant kODErrorCredentialsParameterError is when a parameter provided is invalid
 @constant kODErrorCredentialsOperationFailed is when the requested operation failed (usually due to some unrecoverable error)
 @constant kODErrorCredentialsServerUnreachable is when the authententication server is not reachabable
 @constant kODErrorCredentialsServerNotFound is when the authentication server could not be found for the operation requested
 @constant kODErrorCredentialsServerError is when the authentication server encountered an error
 @constant kODErrorCredentialsServerTimeout is when the authentication server timed out
 @constant kODErrorCredentialsContactMaster is when the authentication server is not the master and the operation requires the master
 @constant kODErrorCredentialsServerCommunicationError is when the authentication server had a communications error
 @constant kODErrorCredentialsAccountNotFound is when the authentication server could not find the account provided
 @constant kODErrorCredentialsAccountDisabled is when the account is disabled
 @constant kODErrorCredentialsAccountExpired is when the account is expired
 @constant kODErrorCredentialsAccountInactive is when the account is inactive
 @constant kODErrorCredentialsAccountTemporarilyLocked is when the account is in backoff (verification attempts ignored for a period of time)
 @constant kODErrorCredentialsAccountLocked is when the account is locked due to too many verification failures.
 @constant kODErrorCredentialsPasswordExpired is when the password has expired and must be changed
 @constant kODErrorCredentialsPasswordChangeRequired is when a password change is required
 @constant kODErrorCredentialsPasswordQualityFailed is when a password provided for change did not meet quality minimum requirements
 @constant kODErrorCredentialsPasswordTooShort is when a password provided is too short
 @constant kODErrorCredentialsPasswordTooLong is when a password provided is too long
 @constant kODErrorCredentialsPasswordNeedsLetter is when a password needs a letter
 @constant kODErrorCredentialsPasswordNeedsDigit is when a password needs a digit
 @constant kODErrorCredentialsPasswordChangeTooSoon is when a an attempt to change a password too soon before last change
 @constant kODErrorCredentialsPasswordUnrecoverable is when password was not recoverable from the authentication database
 @constant kODErrorCredentialsInvalidLogonHours is when an account attempts to login outside of set logon hours
 @constant kODErrorCredentialsInvalidComputer is when an account attempts to login to a computer they are not authorized
 @constant kODErrorPolicyUnsupported all requested policies were not supported
 @constant kODErrorPolicyOutOfRange policy value was beyond the allowed range
 @constant kODErrorPluginOperationNotSupported is when a plugin does not support the requested operation
 @constant kODErrorPluginError is when a plugin has encountered some undefined error
 @constant kODErrorDaemonError is when some error occurred inside the daemon
 @constant kODErrorPluginOperationTimeout is when an operation exceeds an imposed timeout
*/

enum ODFrameworkErrors
{
    kODErrorSuccess                             = 0,
    
	/* ODSession error codes */
	kODErrorSessionLocalOnlyDaemonInUse			=	1000,
	kODErrorSessionNormalDaemonInUse			=	1001,
	kODErrorSessionDaemonNotRunning				=	1002,
	kODErrorSessionDaemonRefused				=	1003,

	kODErrorSessionProxyCommunicationError		=	1100,
	kODErrorSessionProxyVersionMismatch			=	1101,
	kODErrorSessionProxyIPUnreachable			=	1102,
	kODErrorSessionProxyUnknownHost				=	1103,
	
	/* ODNode error codes */
	kODErrorNodeUnknownName						=	2000,
	kODErrorNodeUnknownType						=	2001,
	kODErrorNodeDisabled						=	2002,

	kODErrorNodeConnectionFailed				=	2100,
	
	/* ODNode miscellaneous error codes */
	kODErrorNodeUnknownHost						=	2200,
	
	/* ODQuery error codes */
	kODErrorQuerySynchronize					=	3000,
	
	kODErrorQueryInvalidMatchType				=	3100,
	kODErrorQueryUnsupportedMatchType			=	3101,
	kODErrorQueryTimeout						=	3102,
	
	/* ODRecord error codes */
	kODErrorRecordReadOnlyNode					=	4000,
	kODErrorRecordPermissionError				=	4001,
	
	kODErrorRecordParameterError				=	4100,
	kODErrorRecordInvalidType					=	4101,
	kODErrorRecordAlreadyExists					=	4102,
	kODErrorRecordTypeDisabled					=	4103,
	kODErrorRecordNoLongerExists				=	4104,
	
	kODErrorRecordAttributeUnknownType			=	4200,
	kODErrorRecordAttributeNotFound				=	4201,
	kODErrorRecordAttributeValueSchemaError		=	4202,
	kODErrorRecordAttributeValueNotFound		=   4203,
	
	/* Credential specific error codes */
	kODErrorCredentialsInvalid					=	5000,
	
	kODErrorCredentialsMethodNotSupported		=	5100,
	kODErrorCredentialsNotAuthorized			=	5101,
	kODErrorCredentialsParameterError			=	5102,
	kODErrorCredentialsOperationFailed			=	5103,
	
	kODErrorCredentialsServerUnreachable		=	5200,
	kODErrorCredentialsServerNotFound			=	5201,
	kODErrorCredentialsServerError				=	5202,
	kODErrorCredentialsServerTimeout			=	5203,
	kODErrorCredentialsContactMaster			=	5204,
	kODErrorCredentialsServerCommunicationError	=	5205,
	
	kODErrorCredentialsAccountNotFound			=	5300,
	kODErrorCredentialsAccountDisabled			=	5301,
	kODErrorCredentialsAccountExpired			=	5302,
	kODErrorCredentialsAccountInactive			=	5303,
    kODErrorCredentialsAccountTemporarilyLocked =   5304,
    kODErrorCredentialsAccountLocked            =   5305,

	kODErrorCredentialsPasswordExpired			=	5400,
	kODErrorCredentialsPasswordChangeRequired	=	5401,
	kODErrorCredentialsPasswordQualityFailed	=	5402,
	kODErrorCredentialsPasswordTooShort			=	5403,
	kODErrorCredentialsPasswordTooLong			=	5404,
	kODErrorCredentialsPasswordNeedsLetter		=	5405,
	kODErrorCredentialsPasswordNeedsDigit		=	5406,
	kODErrorCredentialsPasswordChangeTooSoon	=	5407,
	kODErrorCredentialsPasswordUnrecoverable	=	5408,

	kODErrorCredentialsInvalidLogonHours		= 	5500,
	kODErrorCredentialsInvalidComputer			= 	5501,
    
    /* Policy errors */
    kODErrorPolicyUnsupported                   =   6000,
    kODErrorPolicyOutOfRange                    =   6001,

	/* Underlying plugin errors */
	kODErrorPluginOperationNotSupported			=	10000,
	kODErrorPluginError							=	10001,
	kODErrorDaemonError							=	10002,
    kODErrorPluginOperationTimeout              =   10003,
};

/*!
    @const      kODBackOffSeconds
    @abstract   Number of seconds an account is in backoff
    @discussion Returned by verifyPassword and authenticationAllowed APIs.
                This is the number of seconds the account is in "back off".
                During backoff authentication attempts are ignored.
 */
CF_EXPORT
ODErrorUserInfoKeyType kODBackOffSeconds __OSX_AVAILABLE_STARTING(__MAC_10_15, __IPHONE_NA);

#endif
