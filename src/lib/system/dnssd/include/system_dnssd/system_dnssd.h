/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#ifndef _system_dnssd_H_
#define _system_dnssd_H_

void* DNSServiceAddRecord(void);
void* DNSServiceBrowse(void);
void* DNSServiceBrowserCreate(void);
void* DNSServiceConstructFullName(void);
void* DNSServiceCreateConnection(void);
void* DNSServiceCreateDelegateConnection(void);
void* DNSServiceDiscoveryDeallocate(void);
void* DNSServiceDiscoveryMachPort(void);
void* DNSServiceDiscovery_handleReply(void);
void* DNSServiceEnumerateDomains(void);
void* DNSServiceGetAddrInfo(void);
void* DNSServiceGetPID(void);
void* DNSServiceGetProperty(void);
void* DNSServiceNATPortMappingCreate(void);
void* DNSServiceProcessResult(void);
void* DNSServiceQueryRecord(void);
void* DNSServiceReconfirmRecord(void);
void* DNSServiceRefDeallocate(void);
void* DNSServiceRefSockFD(void);
void* DNSServiceRegister(void);
void* DNSServiceRegisterRecord(void);
void* DNSServiceRemoveRecord(void);
void* DNSServiceResolve(void);
void* DNSServiceResolverResolve(void);
void* DNSServiceSetDefaultDomainForUser(void);
void* DNSServiceSetDispatchQueue(void);
void* DNSServiceSleepKeepalive(void);
void* DNSServiceUpdateRecord(void);
void* PeerConnectionRelease(void);
void* TXTRecordContainsKey(void);
void* TXTRecordCreate(void);
void* TXTRecordDeallocate(void);
void* TXTRecordGetBytesPtr(void);
void* TXTRecordGetCount(void);
void* TXTRecordGetItemAtIndex(void);
void* TXTRecordGetLength(void);
void* TXTRecordGetValuePtr(void);
void* TXTRecordRemoveValue(void);
void* TXTRecordSetValue(void);

#endif
