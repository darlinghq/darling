/**
 * This file is part of Darling.
 *
 * Copyright (C) 2021 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CRYPTOTOKENKIT_CTKLOGINHELPER_H_
#define _CRYPTOTOKENKIT_CTKLOGINHELPER_H_

#include <CoreFoundation/CoreFoundation.h>

// not sure if these belong here
// also not sure about their content
#define kTKXpcKeyUserName "username"
#define kTkHintContextData "context-data"
#define kTkHintAllPubkeyHashes "all-pubkey-hashes"
#define kTkHintTokenNameHashes "token-name-hashes"
#define kTkHintTokenNameIds "token-name-ids"
#define kTkHintFriendlyNameHashes "friendly-name-hashes"
#define kTkHintFriendlyNames "friendly-names"
#define kTkHintUnlockTokenHashes "unlock-token-hashes"
#define kTkHintUnlockTokenIds "unlock-token-ids"

CFDictionaryRef TKCopyAvailableTokensInfo(uid_t uid, CFDictionaryRef hints);
int TKPerformLogin(uid_t uid, CFStringRef token_id, CFDataRef pubkey_hash, CFStringRef pin, CFStringRef kerberos_principal, CFErrorRef* error);

#endif // _CRYPTOTOKENKIT_CTKLOGINHELPER_H_
