/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#ifndef _SecurityHI_H_
#define _SecurityHI_H_

void* DisposeURLNotifyUPP(void);
void* DisposeURLSystemEventUPP(void);
void* InvokeURLNotifyUPP(void);
void* InvokeURLSystemEventUPP(void);
void* KCAddAppleSharePassword(void);
void* KCAddGenericPassword(void);
void* KCAddInternetPassword(void);
void* KCAddInternetPasswordWithPath(void);
void* KCAddItem(void);
void* KCChangeSettings(void);
void* KCCreateKeychain(void);
void* KCUnlock(void);
void* NewURLNotifyUPP(void);
void* NewURLSystemEventUPP(void);
void* SecChooseIdentity(void);
void* SecChooseIdentityAsSheet(void);
void* SecDisplayCertificate(void);
void* SecDisplayCertificateGroup(void);
void* SecEditTrust(void);
void* SecEditTrustAsSheet(void);
void* kcaddapplesharepassword(void);
void* kcaddgenericpassword(void);
void* kcaddinternetpassword(void);
void* kcaddinternetpasswordwithpath(void);
void* kccreatekeychain(void);
void* kcunlock(void);

#endif
