/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel
    
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
__attribute__((constructor)) static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}
void* SZArchiverConvertZipArchiveToStreamable(void) { if (verbose) puts("STUB: SZArchiverConvertZipArchiveToStreamable called"); return NULL; }
void* SZArchiverCopyStatsDescriptions(void) { if (verbose) puts("STUB: SZArchiverCopyStatsDescriptions called"); return NULL; }
void* SZArchiverCopyStatsKeys(void) { if (verbose) puts("STUB: SZArchiverCopyStatsKeys called"); return NULL; }
void* SZArchiverCreateStreamableZip(void) { if (verbose) puts("STUB: SZArchiverCreateStreamableZip called"); return NULL; }
void* SZArchiverExtractFile(void) { if (verbose) puts("STUB: SZArchiverExtractFile called"); return NULL; }
void* _SZLog(void) { if (verbose) puts("STUB: _SZLog called"); return NULL; }
