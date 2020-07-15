/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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


#include <TrustedPeers/TrustedPeers.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* TPMachineIDStatusToString(void)
{
    if (verbose) puts("STUB: TPMachineIDStatusToString called");
    return NULL;
}

void* TPModelLog(void)
{
    if (verbose) puts("STUB: TPModelLog called");
    return NULL;
}

void* TPPBAncientEpochReadFrom(void)
{
    if (verbose) puts("STUB: TPPBAncientEpochReadFrom called");
    return NULL;
}

void* TPPBDictionaryMatchingRuleFieldExistsReadFrom(void)
{
    if (verbose) puts("STUB: TPPBDictionaryMatchingRuleFieldExistsReadFrom called");
    return NULL;
}

void* TPPBDictionaryMatchingRuleFieldRegexMatchReadFrom(void)
{
    if (verbose) puts("STUB: TPPBDictionaryMatchingRuleFieldRegexMatchReadFrom called");
    return NULL;
}

void* TPPBDictionaryMatchingRuleReadFrom(void)
{
    if (verbose) puts("STUB: TPPBDictionaryMatchingRuleReadFrom called");
    return NULL;
}

void* TPPBDispositionDisallowedMachineIDReadFrom(void)
{
    if (verbose) puts("STUB: TPPBDispositionDisallowedMachineIDReadFrom called");
    return NULL;
}

void* TPPBDispositionDuplicateMachineIDReadFrom(void)
{
    if (verbose) puts("STUB: TPPBDispositionDuplicateMachineIDReadFrom called");
    return NULL;
}

void* TPPBDispositionEntryReadFrom(void)
{
    if (verbose) puts("STUB: TPPBDispositionEntryReadFrom called");
    return NULL;
}

void* TPPBDispositionReadFrom(void)
{
    if (verbose) puts("STUB: TPPBDispositionReadFrom called");
    return NULL;
}

void* TPPBPeerDynamicInfoReadFrom(void)
{
    if (verbose) puts("STUB: TPPBPeerDynamicInfoReadFrom called");
    return NULL;
}

void* TPPBPeerPermanentInfoReadFrom(void)
{
    if (verbose) puts("STUB: TPPBPeerPermanentInfoReadFrom called");
    return NULL;
}

void* TPPBPeerStableInfoReadFrom(void)
{
    if (verbose) puts("STUB: TPPBPeerStableInfoReadFrom called");
    return NULL;
}

void* TPPBPolicyCategoriesByViewReadFrom(void)
{
    if (verbose) puts("STUB: TPPBPolicyCategoriesByViewReadFrom called");
    return NULL;
}

void* TPPBPolicyDocumentReadFrom(void)
{
    if (verbose) puts("STUB: TPPBPolicyDocumentReadFrom called");
    return NULL;
}

void* TPPBPolicyIntroducersByCategoryReadFrom(void)
{
    if (verbose) puts("STUB: TPPBPolicyIntroducersByCategoryReadFrom called");
    return NULL;
}

void* TPPBPolicyKeyViewMappingReadFrom(void)
{
    if (verbose) puts("STUB: TPPBPolicyKeyViewMappingReadFrom called");
    return NULL;
}

void* TPPBPolicyModelToCategoryReadFrom(void)
{
    if (verbose) puts("STUB: TPPBPolicyModelToCategoryReadFrom called");
    return NULL;
}

void* TPPBPolicyProhibitsReadFrom(void)
{
    if (verbose) puts("STUB: TPPBPolicyProhibitsReadFrom called");
    return NULL;
}

void* TPPBPolicyRedactionReadFrom(void)
{
    if (verbose) puts("STUB: TPPBPolicyRedactionReadFrom called");
    return NULL;
}

void* TPPBPolicySecretReadFrom(void)
{
    if (verbose) puts("STUB: TPPBPolicySecretReadFrom called");
    return NULL;
}

void* TPPBUnknownMachineIDReadFrom(void)
{
    if (verbose) puts("STUB: TPPBUnknownMachineIDReadFrom called");
    return NULL;
}

void* TPPBVoucherReadFrom(void)
{
    if (verbose) puts("STUB: TPPBVoucherReadFrom called");
    return NULL;
}

void* TPPeerStatusToString(void)
{
    if (verbose) puts("STUB: TPPeerStatusToString called");
    return NULL;
}

void* checkTypesafeSignature(void)
{
    if (verbose) puts("STUB: checkTypesafeSignature called");
    return NULL;
}

void* typesafeSignature(void)
{
    if (verbose) puts("STUB: typesafeSignature called");
    return NULL;
}
