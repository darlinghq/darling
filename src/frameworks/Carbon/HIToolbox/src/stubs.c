/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

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

#include <stdio.h>
#include <stdlib.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
}

int AEProcessEvent(void *event)
{
	if (verbose) puts("STUB: AEProcessEvent called");
	return 0;
}

int EnableSecureEventInput(void)
{
	if (verbose) puts("STUB: EnableSecureEventInput called");
	return 0;
}

int DisableSecureEventInput(void)
{
	if (verbose) puts("STUB: DisableSecureEventInput called");
	return 0;
}

unsigned char LMGetKbdLast(void)
{
	if (verbose) puts("STUB: LMGetKbdLast called");
	return 0;
}

void* TISCreateASCIICapableInputSourceList(void)
{
	if (verbose) puts("STUB: TISCreateASCIICapableInputSourceList called");
	return NULL;
}

int TSMSetDocumentProperty(void *docID, unsigned int propertyTag, unsigned long size, const void *propertyData)
{
	if (verbose) puts("STUB: TSMSetDocumentProperty called");
	return 0;
}

int TSMRemoveDocumentProperty(void *docID, unsigned int propertyTag)
{
	if (verbose) puts("STUB: TSMRemoveDocumentProperty called");
	return 0;
}
