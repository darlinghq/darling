/*
 * Copyright (c) 2005-2009 Apple Inc. All rights reserved.
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

#include <CoreFoundation/CFString.h>

#define OD_ATTR_ASSIGN(x) CFStringRef kODAttributeType ## x = CFSTR("dsAttrTypeStandard:" # x)

#pragma mark Record Types

CFStringRef kODRecordTypeAttributeTypes				= CFSTR("dsRecTypeStandard:AttributeTypes");
CFStringRef kODRecordTypeAFPServer					= CFSTR("dsRecTypeStandard:AFPServer");
CFStringRef kODRecordTypeAliases					= CFSTR("dsRecTypeStandard:Aliases");
CFStringRef kODRecordTypeAugments					= CFSTR("dsRecTypeStandard:Augments");
CFStringRef kODRecordTypeAutomount					= CFSTR("dsRecTypeStandard:Automount");
CFStringRef kODRecordTypeAutomountMap				= CFSTR("dsRecTypeStandard:AutomountMap");
CFStringRef kODRecordTypeAutoServerSetup			= CFSTR("dsRecTypeStandard:AutoServerSetup");
CFStringRef kODRecordTypeBootp						= CFSTR("dsRecTypeStandard:Bootp");
CFStringRef kODRecordTypeCertificateAuthorities		= CFSTR("dsRecTypeStandard:CertificateAuthorities");
CFStringRef kODRecordTypeComputerLists				= CFSTR("dsRecTypeStandard:ComputerLists");
CFStringRef kODRecordTypeComputerGroups				= CFSTR("dsRecTypeStandard:ComputerGroups");
CFStringRef kODRecordTypeComputers					= CFSTR("dsRecTypeStandard:Computers");
CFStringRef kODRecordTypeConfiguration				= CFSTR("dsRecTypeStandard:Config");
CFStringRef kODRecordTypeEthernets					= CFSTR("dsRecTypeStandard:Ethernets");
CFStringRef kODRecordTypeFileMakerServers			= CFSTR("dsRecTypeStandard:FileMakerServers");
CFStringRef kODRecordTypeFTPServer					= CFSTR("dsRecTypeStandard:FTPServer");
CFStringRef kODRecordTypeGroups						= CFSTR("dsRecTypeStandard:Groups");
CFStringRef kODRecordTypeHostServices				= CFSTR("dsRecTypeStandard:HostServices");
CFStringRef kODRecordTypeHosts						= CFSTR("dsRecTypeStandard:Hosts");
CFStringRef kODRecordTypeLDAPServer					= CFSTR("dsRecTypeStandard:LDAPServer");
CFStringRef kODRecordTypeLocations					= CFSTR("dsRecTypeStandard:Locations");
CFStringRef kODRecordTypeMounts						= CFSTR("dsRecTypeStandard:Mounts");
CFStringRef kODRecordTypeNFS						= CFSTR("dsRecTypeStandard:NFS");
CFStringRef kODRecordTypeNetDomains					= CFSTR("dsRecTypeStandard:NetDomains");
CFStringRef kODRecordTypeNetGroups					= CFSTR("dsRecTypeStandard:NetGroups");
CFStringRef kODRecordTypeNetworks					= CFSTR("dsRecTypeStandard:Networks");
CFStringRef kODRecordTypePasswordServer				= CFSTR("dsRecTypeStandard:PasswordServer");
CFStringRef kODRecordTypePeople						= CFSTR("dsRecTypeStandard:People");
CFStringRef kODRecordTypePlugins					= CFSTR("dsRecTypeStandard:Plugins");
CFStringRef kODRecordTypePresetComputers			= CFSTR("dsRecTypeStandard:PresetComputers");
CFStringRef kODRecordTypePresetComputerGroups		= CFSTR("dsRecTypeStandard:PresetComputerGroups");
CFStringRef kODRecordTypePresetComputerLists		= CFSTR("dsRecTypeStandard:PresetComputerLists");
CFStringRef kODRecordTypePresetGroups				= CFSTR("dsRecTypeStandard:PresetGroups");
CFStringRef kODRecordTypePresetUsers				= CFSTR("dsRecTypeStandard:PresetUsers");
CFStringRef kODRecordTypePrintService				= CFSTR("dsRecTypeStandard:PrintService");
CFStringRef kODRecordTypePrintServiceUser			= CFSTR("dsRecTypeStandard:PrintServiceUser");
CFStringRef kODRecordTypePrinters					= CFSTR("dsRecTypeStandard:Printers");
CFStringRef kODRecordTypeProtocols					= CFSTR("dsRecTypeStandard:Protocols");
CFStringRef kODRecordTypeQTSServer					= CFSTR("dsRecTypeStandard:QTSServer");
CFStringRef kODRecordTypeRecordTypes				= CFSTR("dsRecTypeStandard:RecordTypes");
CFStringRef kODRecordTypeResources					= CFSTR("dsRecTypeStandard:Resources");
CFStringRef kODRecordTypeRPC						= CFSTR("dsRecTypeStandard:RPC");
CFStringRef kODRecordTypeSMBServer					= CFSTR("dsRecTypeStandard:SMBServer");
CFStringRef kODRecordTypeServer						= CFSTR("dsRecTypeStandard:Server");
CFStringRef kODRecordTypeServices					= CFSTR("dsRecTypeStandard:Services");
CFStringRef kODRecordTypeSharePoints				= CFSTR("dsRecTypeStandard:SharePoints");
CFStringRef kODRecordTypeUserAuthenticationData		= CFSTR("dsRecTypeStandard:UserAuthenticationData");
CFStringRef kODRecordTypeUsers						= CFSTR("dsRecTypeStandard:Users");
CFStringRef kODRecordTypeWebServer					= CFSTR("dsRecTypeStandard:WebServer");

#pragma mark Attribute Types

CFStringRef kODAttributeTypeAllAttributes				= CFSTR("dsAttributesAll");
CFStringRef kODAttributeTypeStandardOnly				= CFSTR("dsAttributesStandardAll");
CFStringRef kODAttributeTypeNativeOnly					= CFSTR("dsAttributesNativeAll");

CFStringRef kODAttributeTypeMetaAmbiguousName			= CFSTR("dsAttrTypeStandard:AppleMetaAmbiguousName");
CFStringRef kODAttributeTypeMetaAugmentedAttributes		= CFSTR("dsAttrTypeStandard:AppleMetaAugmentedAttributes");
CFStringRef kODAttributeTypeMetaRecordName				= CFSTR("dsAttrTypeStandard:AppleMetaRecordName");

CFStringRef kODAttributeTypeAdminLimits					= CFSTR("dsAttrTypeStandard:AdminLimits");
CFStringRef kODAttributeTypeAuthenticationHint			= CFSTR("dsAttrTypeStandard:AuthenticationHint");
CFStringRef kODAttributeTypeAllTypes					= CFSTR("dsAttrTypeStandard:AttributeTypes");
CFStringRef kODAttributeTypeAuthorityRevocationList		= CFSTR("dsAttrTypeStandard:AuthorityRevocationList");
CFStringRef kODAttributeTypeBirthday					= CFSTR("dsAttrTypeStandard:Birthday");
CFStringRef kODAttributeTypeCACertificate				= CFSTR("dsAttrTypeStandard:CACertificate");
CFStringRef kODAttributeTypeCapacity					= CFSTR("dsAttrTypeStandard:Capacity");
CFStringRef kODAttributeTypeCategory					= CFSTR("dsAttrTypeStandard:Category");
CFStringRef kODAttributeTypeCertificateRevocationList	= CFSTR("dsAttrTypeStandard:CertificateRevocationList");
CFStringRef kODAttributeTypeComment						= CFSTR("dsAttrTypeStandard:Comment");
CFStringRef kODAttributeTypeContactGUID					= CFSTR("dsAttrTypeStandard:ContactGUID");
CFStringRef kODAttributeTypeContactPerson				= CFSTR("dsAttrTypeStandard:ContactPerson");
CFStringRef kODAttributeTypeCreationTimestamp			= CFSTR("dsAttrTypeStandard:CreationTimestamp");
CFStringRef kODAttributeTypeCrossCertificatePair		= CFSTR("dsAttrTypeStandard:CrossCertificatePair");
CFStringRef kODAttributeTypeDataStamp					= CFSTR("dsAttrTypeStandard:DataStamp");
CFStringRef kODAttributeTypeFullName					= CFSTR("dsAttrTypeStandard:RealName");
CFStringRef kODAttributeTypeDNSDomain					= CFSTR("dsAttrTypeStandard:DNSDomain");
CFStringRef kODAttributeTypeDNSNameServer				= CFSTR("dsAttrTypeStandard:DNSNameServer");
CFStringRef kODAttributeTypeENetAddress					= CFSTR("dsAttrTypeStandard:ENetAddress");
CFStringRef kODAttributeTypeExpire						= CFSTR("dsAttrTypeStandard:Expire");
CFStringRef kODAttributeTypeFirstName					= CFSTR("dsAttrTypeStandard:FirstName");
CFStringRef kODAttributeTypeGUID						= CFSTR("dsAttrTypeStandard:GeneratedUID");
CFStringRef kODAttributeTypeHomeDirectoryQuota			= CFSTR("dsAttrTypeStandard:HomeDirectoryQuota");
CFStringRef kODAttributeTypeHomeDirectorySoftQuota		= CFSTR("dsAttrTypeStandard:HomeDirectorySoftQuota");
CFStringRef kODAttributeTypeHomeLocOwner				= CFSTR("dsAttrTypeStandard:HomeLocOwner");
CFStringRef kODAttributeTypeInternetAlias				= CFSTR("dsAttrTypeStandard:InetAlias");
CFStringRef kODAttributeTypeKDCConfigData				= CFSTR("dsAttrTypeStandard:KDCConfigData");
CFStringRef kODAttributeTypeLastName					= CFSTR("dsAttrTypeStandard:LastName");
CFStringRef kODAttributeTypeLDAPSearchBaseSuffix		= CFSTR("dsAttrTypeStandard:LDAPSearchBaseSuffix");
CFStringRef kODAttributeTypeLocation					= CFSTR("dsAttrTypeStandard:Location");
CFStringRef kODAttributeTypeMapGUID						= CFSTR("dsAttrTypeStandard:MapGUID");
CFStringRef kODAttributeTypeMCXFlags					= CFSTR("dsAttrTypeStandard:MCXFlags");
CFStringRef kODAttributeTypeMCXSettings					= CFSTR("dsAttrTypeStandard:MCXSettings");
CFStringRef kODAttributeTypeMailAttribute				= CFSTR("dsAttrTypeStandard:MailAttribute");
CFStringRef kODAttributeTypeMetaAutomountMap			= CFSTR("dsAttrTypeStandard:MetaAutomountMap");
CFStringRef kODAttributeTypeMiddleName					= CFSTR("dsAttrTypeStandard:MiddleName");
CFStringRef kODAttributeTypeModificationTimestamp		= CFSTR("dsAttrTypeStandard:ModificationTimestamp");
CFStringRef kODAttributeTypeNFSHomeDirectory			= CFSTR("dsAttrTypeStandard:NFSHomeDirectory");
CFStringRef kODAttributeTypeNote						= CFSTR("dsAttrTypeStandard:Note");
CFStringRef kODAttributeTypeOwner						= CFSTR("dsAttrTypeStandard:Owner");
CFStringRef kODAttributeTypeOwnerGUID					= CFSTR("dsAttrTypeStandard:OwnerGUID");
CFStringRef kODAttributeTypePassword					= CFSTR("dsAttrTypeStandard:Password");
CFStringRef kODAttributeTypePasswordPlus				= CFSTR("dsAttrTypeStandard:PasswordPlus");
CFStringRef kODAttributeTypePasswordPolicyOptions		= CFSTR("dsAttrTypeStandard:PasswordPolicyOptions");
CFStringRef kODAttributeTypePasswordServerList			= CFSTR("dsAttrTypeStandard:PasswordServerList");
CFStringRef kODAttributeTypePasswordServerLocation		= CFSTR("dsAttrTypeStandard:PasswordServerLocation");
CFStringRef kODAttributeTypePicture						= CFSTR("dsAttrTypeStandard:Picture");
CFStringRef kODAttributeTypePort						= CFSTR("dsAttrTypeStandard:Port");
CFStringRef kODAttributeTypePresetUserIsAdmin			= CFSTR("dsAttrTypeStandard:PresetUserIsAdmin");
CFStringRef kODAttributeTypePrimaryComputerGUID			= CFSTR("dsAttrTypeStandard:PrimaryComputerGUID");
CFStringRef kODAttributeTypePrimaryComputerList			= CFSTR("dsAttrTypeStandard:PrimaryComputerList");
CFStringRef kODAttributeTypePrimaryGroupID				= CFSTR("dsAttrTypeStandard:PrimaryGroupID");
CFStringRef kODAttributeTypePrinter1284DeviceID			= CFSTR("dsAttrTypeStandard:Printer1284DeviceID");
CFStringRef kODAttributeTypePrinterLPRHost				= CFSTR("dsAttrTypeStandard:PrinterLPRHost");
CFStringRef kODAttributeTypePrinterLPRQueue				= CFSTR("dsAttrTypeStandard:PrinterLPRQueue");
CFStringRef kODAttributeTypePrinterMakeAndModel			= CFSTR("dsAttrTypeStandard:PrinterMakeAndModel");
CFStringRef kODAttributeTypePrinterType					= CFSTR("dsAttrTypeStandard:PrinterType");
CFStringRef kODAttributeTypePrinterURI					= CFSTR("dsAttrTypeStandard:PrinterURI");
CFStringRef kODAttributeTypePrinterXRISupported			= CFSTR("dsAttrTypeStandard:PrinterXRISupported");
CFStringRef kODAttributeTypePrintServiceInfoText		= CFSTR("dsAttrTypeStandard:PrintServiceInfoText");
CFStringRef kODAttributeTypePrintServiceInfoXML			= CFSTR("dsAttrTypeStandard:PrintServiceInfoXML");
CFStringRef kODAttributeTypePrintServiceUserData		= CFSTR("dsAttrTypeStandard:PrintServiceUserData");
CFStringRef kODAttributeTypeRealUserID					= CFSTR("dsAttrTypeStandard:RealUserID");
CFStringRef kODAttributeTypeRelativeDNPrefix			= CFSTR("dsAttrTypeStandard:RelativeDNPrefix");
CFStringRef kODAttributeTypeSMBAcctFlags				= CFSTR("dsAttrTypeStandard:SMBAccountFlags");
CFStringRef kODAttributeTypeSMBGroupRID					= CFSTR("dsAttrTypeStandard:SMBGroupRID");
CFStringRef kODAttributeTypeSMBHome						= CFSTR("dsAttrTypeStandard:SMBHome");
CFStringRef kODAttributeTypeSMBHomeDrive				= CFSTR("dsAttrTypeStandard:SMBHomeDrive");
CFStringRef kODAttributeTypeSMBKickoffTime				= CFSTR("dsAttrTypeStandard:SMBKickoffTime");
CFStringRef kODAttributeTypeSMBLogoffTime				= CFSTR("dsAttrTypeStandard:SMBLogoffTime");
CFStringRef kODAttributeTypeSMBLogonTime				= CFSTR("dsAttrTypeStandard:SMBLogonTime");
CFStringRef kODAttributeTypeSMBPrimaryGroupSID			= CFSTR("dsAttrTypeStandard:SMBPrimaryGroupSID");
CFStringRef kODAttributeTypeSMBPWDLastSet				= CFSTR("dsAttrTypeStandard:SMBPasswordLastSet");
CFStringRef kODAttributeTypeSMBProfilePath				= CFSTR("dsAttrTypeStandard:SMBProfilePath");
CFStringRef kODAttributeTypeSMBRID						= CFSTR("dsAttrTypeStandard:SMBRID");
CFStringRef kODAttributeTypeSMBScriptPath				= CFSTR("dsAttrTypeStandard:SMBScriptPath");
CFStringRef kODAttributeTypeSMBSID						= CFSTR("dsAttrTypeStandard:SMBSID");
CFStringRef kODAttributeTypeSMBUserWorkstations			= CFSTR("dsAttrTypeStandard:SMBUserWorkstations");
CFStringRef kODAttributeTypeServiceType					= CFSTR("dsAttrTypeStandard:ServiceType");
CFStringRef kODAttributeTypeSetupAdvertising			= CFSTR("dsAttrTypeStandard:SetupAssistantAdvertising");
CFStringRef kODAttributeTypeSetupAutoRegister			= CFSTR("dsAttrTypeStandard:SetupAssistantAutoRegister");
CFStringRef kODAttributeTypeSetupLocation				= CFSTR("dsAttrTypeStandard:SetupAssistantLocation");
CFStringRef kODAttributeTypeSetupOccupation				= CFSTR("dsAttrTypeStandard:Occupation");
CFStringRef kODAttributeTypeTimeToLive					= CFSTR("dsAttrTypeStandard:TimeToLive");
CFStringRef kODAttributeTypeTrustInformation			= CFSTR("dsAttrTypeStandard:TrustInformation");
CFStringRef kODAttributeTypeUniqueID					= CFSTR("dsAttrTypeStandard:UniqueID");
CFStringRef kODAttributeTypeUserCertificate				= CFSTR("dsAttrTypeStandard:UserCertificate");
CFStringRef kODAttributeTypeUserPKCS12Data				= CFSTR("dsAttrTypeStandard:UserPKCS12Data");
CFStringRef kODAttributeTypeUserShell					= CFSTR("dsAttrTypeStandard:UserShell");
CFStringRef kODAttributeTypeUserSMIMECertificate		= CFSTR("dsAttrTypeStandard:UserSMIMECertificate");
CFStringRef kODAttributeTypeVFSDumpFreq					= CFSTR("dsAttrTypeStandard:VFSDumpFreq");
CFStringRef kODAttributeTypeVFSLinkDir					= CFSTR("dsAttrTypeStandard:VFSLinkDir");
CFStringRef kODAttributeTypeVFSPassNo					= CFSTR("dsAttrTypeStandard:VFSPassNo");
CFStringRef kODAttributeTypeVFSType						= CFSTR("dsAttrTypeStandard:VFSType");
CFStringRef kODAttributeTypeWeblogURI					= CFSTR("dsAttrTypeStandard:WeblogURI");
CFStringRef kODAttributeTypeXMLPlist					= CFSTR("dsAttrTypeStandard:XMLPlist");
CFStringRef kODAttributeTypeProtocolNumber				= CFSTR("dsAttrTypeStandard:ProtocolNumber");
CFStringRef kODAttributeTypeRPCNumber					= CFSTR("dsAttrTypeStandard:RPCNumber");
CFStringRef kODAttributeTypeNetworkNumber				= CFSTR("dsAttrTypeStandard:NetworkNumber");
CFStringRef kODAttributeTypeAccessControlEntry			= CFSTR("dsAttrTypeStandard:AccessControlEntry");
CFStringRef kODAttributeTypeAddressLine1				= CFSTR("dsAttrTypeStandard:AddressLine1");
CFStringRef kODAttributeTypeAddressLine2				= CFSTR("dsAttrTypeStandard:AddressLine2");
CFStringRef kODAttributeTypeAddressLine3				= CFSTR("dsAttrTypeStandard:AddressLine3");
CFStringRef kODAttributeTypeAreaCode					= CFSTR("dsAttrTypeStandard:AreaCode");
CFStringRef kODAttributeTypeAuthenticationAuthority		= CFSTR("dsAttrTypeStandard:AuthenticationAuthority");
CFStringRef kODAttributeTypeAutomountInformation		= CFSTR("dsAttrTypeStandard:AutomountInformation");
CFStringRef kODAttributeTypeBootParams					= CFSTR("dsAttrTypeStandard:BootParams");
CFStringRef kODAttributeTypeBuilding					= CFSTR("dsAttrTypeStandard:Building");
CFStringRef kODAttributeTypeServicesLocator				= CFSTR("dsAttrTypeStandard:ServicesLocator");
CFStringRef kODAttributeTypeCity						= CFSTR("dsAttrTypeStandard:City");
CFStringRef kODAttributeTypeCompany						= CFSTR("dsAttrTypeStandard:Company");
CFStringRef kODAttributeTypeComputers					= CFSTR("dsAttrTypeStandard:Computers");
CFStringRef kODAttributeTypeCountry						= CFSTR("dsAttrTypeStandard:Country");
CFStringRef kODAttributeTypeDepartment					= CFSTR("dsAttrTypeStandard:Department");
CFStringRef kODAttributeTypeDNSName						= CFSTR("dsAttrTypeStandard:DNSName");
CFStringRef kODAttributeTypeEMailAddress				= CFSTR("dsAttrTypeStandard:EMailAddress");
CFStringRef kODAttributeTypeEMailContacts				= CFSTR("dsAttrTypeStandard:EMailContacts");
CFStringRef kODAttributeTypeFaxNumber					= CFSTR("dsAttrTypeStandard:FAXNumber");
CFStringRef kODAttributeTypeGroup						= CFSTR("dsAttrTypeStandard:Group");
CFStringRef kODAttributeTypeGroupMembers				= CFSTR("dsAttrTypeStandard:GroupMembers");
CFStringRef kODAttributeTypeGroupMembership				= CFSTR("dsAttrTypeStandard:GroupMembership");
CFStringRef kODAttributeTypeGroupServices				= CFSTR("dsAttrTypeStandard:GroupServices");
CFStringRef kODAttributeTypeHomePhoneNumber				= CFSTR("dsAttrTypeStandard:HomePhoneNumber");
CFStringRef kODAttributeTypeHTML						= CFSTR("dsAttrTypeStandard:HTML");
CFStringRef kODAttributeTypeHomeDirectory				= CFSTR("dsAttrTypeStandard:HomeDirectory");
CFStringRef kODAttributeTypeIMHandle					= CFSTR("dsAttrTypeStandard:IMHandle");
CFStringRef kODAttributeTypeIPAddress					= CFSTR("dsAttrTypeStandard:IPAddress");
CFStringRef kODAttributeTypeIPAddressAndENetAddress		= CFSTR("dsAttrTypeStandard:IPAddressAndENetAddress");
CFStringRef kODAttributeTypeIPv6Address					= CFSTR("dsAttrTypeStandard:IPv6Address");
CFStringRef kODAttributeTypeJPEGPhoto					= CFSTR("dsAttrTypeStandard:JPEGPhoto");
CFStringRef kODAttributeTypeJobTitle					= CFSTR("dsAttrTypeStandard:JobTitle");
CFStringRef kODAttributeTypeKDCAuthKey					= CFSTR("dsAttrTypeStandard:KDCAuthKey");
CFStringRef kODAttributeTypeKeywords					= CFSTR("dsAttrTypeStandard:Keywords");
CFStringRef kODAttributeTypeLDAPReadReplicas			= CFSTR("dsAttrTypeStandard:LDAPReadReplicas");
CFStringRef kODAttributeTypeLDAPWriteReplicas			= CFSTR("dsAttrTypeStandard:LDAPWriteReplicas");
CFStringRef kODAttributeTypeMapCoordinates				= CFSTR("dsAttrTypeStandard:MapCoordinates");
CFStringRef kODAttributeTypeMapURI						= CFSTR("dsAttrTypeStandard:MapURI");
CFStringRef kODAttributeTypeMIME						= CFSTR("dsAttrTypeStandard:MIME");
CFStringRef kODAttributeTypeMobileNumber				= CFSTR("dsAttrTypeStandard:MobileNumber");
CFStringRef kODAttributeTypeNestedGroups				= CFSTR("dsAttrTypeStandard:NestedGroups");
CFStringRef kODAttributeTypeNetGroups					= CFSTR("dsAttrTypeStandard:NetGroups");
CFStringRef kODAttributeTypeNickName					= CFSTR("dsAttrTypeStandard:NickName");
CFStringRef kODAttributeTypeOrganizationInfo			= CFSTR("dsAttrTypeStandard:OrganizationInfo");
CFStringRef kODAttributeTypeOrganizationName			= CFSTR("dsAttrTypeStandard:OrganizationName");
CFStringRef kODAttributeTypePagerNumber					= CFSTR("dsAttrTypeStandard:PagerNumber");
CFStringRef kODAttributeTypePhoneContacts				= CFSTR("dsAttrTypeStandard:PhoneContacts");
CFStringRef kODAttributeTypePhoneNumber					= CFSTR("dsAttrTypeStandard:PhoneNumber");
CFStringRef kODAttributeTypePGPPublicKey				= CFSTR("dsAttrTypeStandard:PGPPublicKey");
CFStringRef kODAttributeTypePostalAddress				= CFSTR("dsAttrTypeStandard:PostalAddress");
CFStringRef kODAttributeTypePostalAddressContacts		= CFSTR("dsAttrTypeStandard:PostalAddressContacts");
CFStringRef kODAttributeTypePostalCode					= CFSTR("dsAttrTypeStandard:PostalCode");
CFStringRef kODAttributeTypeNamePrefix					= CFSTR("dsAttrTypeStandard:NamePrefix");
CFStringRef kODAttributeTypeProtocols					= CFSTR("dsAttrTypeStandard:Protocols");
CFStringRef kODAttributeTypeRecordName					= CFSTR("dsAttrTypeStandard:RecordName");
CFStringRef kODAttributeTypeRelationships				= CFSTR("dsAttrTypeStandard:Relationships");
CFStringRef kODAttributeTypeResourceInfo				= CFSTR("dsAttrTypeStandard:ResourceInfo");
CFStringRef kODAttributeTypeResourceType				= CFSTR("dsAttrTypeStandard:ResourceType");
CFStringRef kODAttributeTypeState						= CFSTR("dsAttrTypeStandard:State");
CFStringRef kODAttributeTypeStreet						= CFSTR("dsAttrTypeStandard:Street");
CFStringRef kODAttributeTypeNameSuffix					= CFSTR("dsAttrTypeStandard:NameSuffix");
CFStringRef kODAttributeTypeURL							= CFSTR("dsAttrTypeStandard:URL");
CFStringRef kODAttributeTypeVFSOpts						= CFSTR("dsAttrTypeStandard:VFSOpts");

CFStringRef kODAttributeTypeAlias						= CFSTR("dsAttrTypeStandard:Alias");
CFStringRef kODAttributeTypeAuthCredential				= CFSTR("dsAttrTypeStandard:AuthCredential");
CFStringRef kODAttributeTypeCopyTimestamp				= CFSTR("dsAttrTypeStandard:CopyTimestamp");
CFStringRef kODAttributeTypeDateRecordCreated			= CFSTR("dsAttrTypeStandard:DateRecordCreated");
CFStringRef kODAttributeTypeKerberosRealm				= CFSTR("dsAttrTypeStandard:KerberosRealm");
CFStringRef kODAttributeTypeKerberosServices			= CFSTR("dsAttrTypeStandard:KerberosServices");
CFStringRef kODAttributeTypeNTDomainComputerAccount		= CFSTR("dsAttrTypeStandard:NTDomainComputerAccount");
CFStringRef kODAttributeTypeOriginalHomeDirectory		= CFSTR("dsAttrTypeStandard:OriginalHomeDirectory");
CFStringRef kODAttributeTypeOriginalNFSHomeDirectory	= CFSTR("dsAttrTypeStandard:OriginalNFSHomeDirectory");
CFStringRef kODAttributeTypeOriginalNodeName			= CFSTR("dsAttrTypeStandard:OriginalNodeName");
CFStringRef kODAttributeTypePrimaryNTDomain				= CFSTR("dsAttrTypeStandard:PrimaryNTDomain");
CFStringRef kODAttributeTypePwdAgingPolicy				= CFSTR("dsAttrTypeStandard:PwdAgingPolicy");
CFStringRef kODAttributeTypeReadOnlyNode				= CFSTR("dsAttrTypeStandard:ReadOnlyNode");
CFStringRef kODAttributeTypeTimePackage					= CFSTR("dsAttrTypeStandard:TimePackage");
CFStringRef kODAttributeTypeTotalSize					= CFSTR("dsAttrTypeStandard:TotalSize");
CFStringRef kODAttributeTypeAuthMethod					= CFSTR("dsAttrTypeStandard:AuthMethod");
CFStringRef kODAttributeTypeMetaNodeLocation			= CFSTR("dsAttrTypeStandard:AppleMetaNodeLocation");
CFStringRef kODAttributeTypeNodePath					= CFSTR("dsAttrTypeStandard:NodePath");
CFStringRef kODAttributeTypePlugInInfo					= CFSTR("dsAttrTypeStandard:PlugInInfo");
CFStringRef kODAttributeTypeRecordType					= CFSTR("dsAttrTypeStandard:RecordType");
CFStringRef kODAttributeTypeSchema						= CFSTR("dsAttrTypeStandard:Scheama");
CFStringRef kODAttributeTypeSubNodes					= CFSTR("dsAttrTypeStandard:SubNodes");

CFStringRef kODAttributeTypeNetGroupTriplet				= CFSTR("dsAttrTypeStandard:NetGroupTriplet");

CFStringRef kODAttributeTypeSearchPath					= CFSTR("dsAttrTypeStandard:SearchPath");
CFStringRef kODAttributeTypeSearchPolicy				= CFSTR("dsAttrTypeStandard:SearchPolicy");
CFStringRef kODAttributeTypeAutomaticSearchPath			= CFSTR("dsAttrTypeStandard:NSPSearchPath");
CFStringRef kODAttributeTypeLocalOnlySearchPath			= CFSTR("dsAttrTypeStandard:LSPSearchPath");
CFStringRef kODAttributeTypeCustomSearchPath			= CFSTR("dsAttrTypeStandard:CSPSearchPath");

CFStringRef kODAttributeTypeAdvertisedServices			= CFSTR("dsAttrTypeStandard:AdvertisedServices");
CFStringRef kODAttributeTypeLocaleRelay					= CFSTR("dsAttrTypeStandard:LocaleRelay");
CFStringRef kODAttributeTypeLocaleSubnets				= CFSTR("dsAttrTypeStandard:LocaleSubnets");
CFStringRef kODAttributeTypeNetworkInterfaces			= CFSTR("dsAttrTypeStandard:NetworkInterfaces");
CFStringRef kODAttributeTypeParentLocales				= CFSTR("dsAttrTypeStandard:ParentLocales");
CFStringRef kODAttributeTypePrimaryLocale				= CFSTR("dsAttrTypeStandard:PrimaryLocale");

OD_ATTR_ASSIGN(KerberosACL);
OD_ATTR_ASSIGN(KerberosKeySet);
OD_ATTR_ASSIGN(KerberosPrincipalACL);
OD_ATTR_ASSIGN(KerberosPrincipalAliases);
OD_ATTR_ASSIGN(KerberosPrincipalName);
OD_ATTR_ASSIGN(KerberosTicketPolicy);

#pragma mark Attribute Types (Configure Node)

CFStringRef kODAttributeTypeBuildVersion				= CFSTR("dsAttrTypeStandard:BuildVersion");
CFStringRef kODAttributeTypeConfigAvailable				= CFSTR("dsAttrTypeStandard:ConfigAvail");
CFStringRef kODAttributeTypeConfigFile					= CFSTR("dsAttrTypeStandard:ConfigFile");
CFStringRef kODAttributeTypeCoreFWVersion				= CFSTR("dsAttrTypeStandard:CoreFWVersion");
CFStringRef kODAttributeTypeFunctionalState				= CFSTR("dsAttrTypeStandard:FunctionalState");
CFStringRef kODAttributeTypeFWVersion					= CFSTR("dsAttrTypeStandard:FWVersion");
CFStringRef kODAttributeTypePluginIndex					= CFSTR("dsAttrTypeStandard:PluginIndex");
CFStringRef kODAttributeTypeNumTableList				= CFSTR("dsAttrTypeStandard:RefNumTableList");
CFStringRef kODAttributeTypeVersion						= CFSTR("dsAttrTypeStandard:Version");
CFStringRef kODAttributeTypePIDValue					= CFSTR("dsAttrTypeStandard:PIDValue");
CFStringRef kODAttributeTypeProcessName					= CFSTR("dsAttrTypeStandard:ProcessName");
CFStringRef kODAttributeTypeTotalRefCount				= CFSTR("dsAttrTypeStandard:TotalRefCount");
CFStringRef kODAttributeTypeDirRefCount					= CFSTR("dsAttrTypeStandard:DirRefCount");
CFStringRef kODAttributeTypeNodeRefCount				= CFSTR("dsAttrTypeStandard:NodeRefCount");
CFStringRef kODAttributeTypeRecRefCount					= CFSTR("dsAttrTypeStandard:RecRefCount");
CFStringRef kODAttributeTypeAttrListRefCount			= CFSTR("dsAttrTypeStandard:AttrListRefCount");
CFStringRef kODAttributeTypeAttrListValueRefCount		= CFSTR("dsAttrTypeStandard:AttrListValueRefCount");
CFStringRef kODAttributeTypeDirRefs						= CFSTR("dsAttrTypeStandard:DirRefs");
CFStringRef kODAttributeTypeNodeRefs					= CFSTR("dsAttrTypeStandard:NodeRefs");
CFStringRef kODAttributeTypeRecRefs						= CFSTR("dsAttrTypeStandard:RecRefs");
CFStringRef kODAttributeTypeAttrListRefs				= CFSTR("dsAttrTypeStandard:AttrListRefs");
CFStringRef kODAttributeTypeAttrListValueRefs			= CFSTR("dsAttrTypeStandard:AttrListValueRefs");
CFStringRef kODAttributeTypeOperatingSystem				= CFSTR("dsAttrTypeStandard:OperatingSystem");
CFStringRef kODAttributeTypeOperatingSystemVersion		= CFSTR("dsAttrTypeStandard:OperatingSystemVersion");
CFStringRef kODAttributeTypeAltSecurityIdentities		= CFSTR("dsAttrTypeStandard:AltSecurityIdentities");
CFStringRef kODAttributeTypeHardwareUUID				= CFSTR("dsAttrTypeStandard:HardwareUUID");

#pragma mark Authentication Types

CFStringRef kODAuthenticationType2WayRandom							= CFSTR("dsAuthMethodStandard:dsAuth2WayRandom");
CFStringRef kODAuthenticationType2WayRandomChangePasswd				= CFSTR("dsAuthMethodStandard:dsAuth2WayRandomChangePasswd");
CFStringRef kODAuthenticationTypeAPOP								= CFSTR("dsAuthMethodStandard:dsAuthAPOP");
CFStringRef kODAuthenticationTypeCRAM_MD5							= CFSTR("dsAuthMethodStandard:dsAuthNodeCRAM-MD5");
CFStringRef kODAuthenticationTypeChangePasswd						= CFSTR("dsAuthMethodStandard:dsAuthChangePasswd");
CFStringRef kODAuthenticationTypeClearText							= CFSTR("dsAuthMethodStandard:dsAuthClearText");
CFStringRef kODAuthenticationTypeCrypt								= CFSTR("dsAuthMethodStandard:dsAuthCrypt");
CFStringRef kODAuthenticationTypeDIGEST_MD5							= CFSTR("dsAuthMethodStandard:dsAuthNodeDIGEST-MD5");
CFStringRef kODAuthenticationTypeDeleteUser							= CFSTR("dsAuthMethodStandard:dsAuthDeleteUser");
CFStringRef kODAuthenticationTypeGetEffectivePolicy					= CFSTR("dsAuthMethodStandard:dsAuthGetEffectivePolicy");
CFStringRef kODAuthenticationTypeGetGlobalPolicy					= CFSTR("dsAuthMethodStandard:dsAuthGetGlobalPolicy");
CFStringRef kODAuthenticationTypeGetKerberosPrincipal				= CFSTR("dsAuthMethodStandard:dsAuthGetKerberosPrincipal");
CFStringRef kODAuthenticationTypeGetPolicy							= CFSTR("dsAuthMethodStandard:dsAuthGetPolicy");
CFStringRef kODAuthenticationTypeGetUserData						= CFSTR("dsAuthMethodStandard:dsAuthGetUserData");
CFStringRef kODAuthenticationTypeGetUserName						= CFSTR("dsAuthMethodStandard:dsAuthGetUserName");
CFStringRef kODAuthenticationTypeKerberosTickets					= CFSTR("dsAuthMethodStandard:dsAuthKerberosTickets");
CFStringRef kODAuthenticationTypeMPPEMasterKeys						= CFSTR("dsAuthMethodStandard:dsAuthMPPEMasterKeys");
CFStringRef kODAuthenticationTypeMSCHAP2							= CFSTR("dsAuthMethodStandard:dsAuthMSCHAP2");
CFStringRef kODAuthenticationTypeNTLMv2								= CFSTR("dsAuthMethodStandard:dsAuthNodeNTLMv2");
CFStringRef kODAuthenticationTypeNTLMv2WithSessionKey				= CFSTR("dsAuthMethodStandard:dsAuthNodeNTLMv2WithSessionKey");
CFStringRef kODAuthenticationTypeNewUser							= CFSTR("dsAuthMethodStandard:dsAuthNewUser");
CFStringRef kODAuthenticationTypeNewUserWithPolicy					= CFSTR("dsAuthMethodStandard:dsAuthNewUserWithPolicy");
CFStringRef kODAuthenticationTypeNodeNativeClearTextOK				= CFSTR("dsAuthMethodStandard:dsAuthNodeNativeCanUseClearText");
CFStringRef kODAuthenticationTypeNodeNativeNoClearText				= CFSTR("dsAuthMethodStandard:dsAuthNodeNativeCannotUseClearText");
CFStringRef kODAuthenticationTypeNodeNativeRetainCredential			= CFSTR("dsAuthMethodStandard:dsAuthNodeNativeRetainCredential");
CFStringRef kODAuthenticationTypeNodePPS							= CFSTR("dsAuthMethodStandard:dsAuthNodePPS");
CFStringRef kODAuthenticationTypeReadSecureHash						= CFSTR("dsAuthMethodStandard:dsAuthReadSecureHash");
CFStringRef kODAuthenticationTypeSMBNTv2UserSessionKey				= CFSTR("dsAuthMethodStandard:dsSMBNTv2UserSessionKey");
CFStringRef kODAuthenticationTypeSMBWorkstationCredentialSessionKey	= CFSTR("dsAuthMethodStandard:dsAuthSMBWorkstationCredentialSessionKey");
CFStringRef kODAuthenticationTypeSMB_LM_Key							= CFSTR("dsAuthMethodStandard:dsAuthSMBLMKey");
CFStringRef kODAuthenticationTypeSMB_NT_Key							= CFSTR("dsAuthMethodStandard:dsAuthSMBNTKey");
CFStringRef kODAuthenticationTypeSMB_NT_UserSessionKey				= CFSTR("dsAuthMethodStandard:dsAuthSMBNTUserSessionKey");
CFStringRef kODAuthenticationTypeSMB_NT_WithUserSessionKey			= CFSTR("dsAuthMethodStandard:dsAuthNTWithSessionKey");
CFStringRef kODAuthenticationTypeSecureHash							= CFSTR("dsAuthMethodStandard:dsAuthSecureHash");
CFStringRef kODAuthenticationTypeSetCertificateHashAsCurrent		= CFSTR("dsAuthMethodStandard:dsAuthSetCertificateHashAsRoot");
CFStringRef kODAuthenticationTypeSetGlobalPolicy					= CFSTR("dsAuthMethodStandard:dsAuthSetGlobalPolicy");
CFStringRef kODAuthenticationTypeSetLMHash							= CFSTR("dsAuthMethodStandard:dsAuthSetLMHash");
CFStringRef kODAuthenticationTypeSetNTHash							= CFSTR("dsAuthMethodStandard:dsAuthSetNTHash");
CFStringRef kODAuthenticationTypeSetPassword						= CFSTR("dsAuthMethodStandard:dsAuthSetPasswd");
CFStringRef kODAuthenticationTypeSetPasswordAsCurrent				= CFSTR("dsAuthMethodStandard:dsAuthSetPasswdAsRoot");
CFStringRef kODAuthenticationTypeSetPolicy							= CFSTR("dsAuthMethodStandard:dsAuthSetPolicy");
CFStringRef kODAuthenticationTypeSetPolicyAsCurrent					= CFSTR("dsAuthMethodStandard:dsAuthSetPolicyAsRoot");
CFStringRef kODAuthenticationTypeSetUserData						= CFSTR("dsAuthMethodStandard:dsAuthSetUserData");
CFStringRef kODAuthenticationTypeSetUserName						= CFSTR("dsAuthMethodStandard:dsAuthSetUserName");
CFStringRef kODAuthenticationTypeSetWorkstationPassword				= CFSTR("dsAuthMethodStandard:dsAuthSetWorkstationPasswd");
CFStringRef kODAuthenticationTypeWithAuthorizationRef				= CFSTR("dsAuthMethodStandard:dsAuthWithAuthorizationRef");
CFStringRef kODAuthenticationTypeWriteSecureHash					= CFSTR("dsAuthMethodStandard:dsAuthWriteSecureHash");
