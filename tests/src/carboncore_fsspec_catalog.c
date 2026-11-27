/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

/*
 * FSGetCatalogInfo fills in the optional FSSpec output (32-bit only, as
 * FSSpec is a 32-bit Carbon type). Checked through the File Manager
 * round trip: the spec names the same item as the FSRef it came from.
 */
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;

#define CHECK(x) do { \
    if (!(x)) { \
        fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); \
        failures++; \
    } \
} while (0)

int main(void)
{
    char path[] = "/tmp/fsspec-catalog-XXXXXX";
    int fd = mkstemp(path);
    FSRef ref, back;
    FSSpec spec;
    FSCatalogInfo info;
    const char *leaf = strrchr(path, '/') + 1;

    if (fd < 0)
        return 2;
    close(fd);
    if (FSPathMakeRef((const UInt8 *)path, &ref, NULL) != noErr) {
        unlink(path);
        return 2;
    }

    memset(&spec, 0xaa, sizeof(spec));
    memset(&info, 0, sizeof(info));
    CHECK(FSGetCatalogInfo(&ref, kFSCatInfoParentDirID, &info, NULL,
                           &spec, NULL) == noErr);
    CHECK(spec.name[0] == strlen(leaf));
    CHECK(memcmp(spec.name + 1, leaf, strlen(leaf)) == 0);
    CHECK(spec.parID == (SInt32)info.parentDirID);
    CHECK(spec.vRefNum != 0xaaaa);

    /* The spec resolves back to the same item. */
    CHECK(FSpMakeFSRef(&spec, &back) == noErr);
    CHECK(FSCompareFSRefs(&ref, &back) == noErr);

    /* Asking for the spec alone works too. */
    memset(&spec, 0, sizeof(spec));
    CHECK(FSGetCatalogInfo(&ref, 0, NULL, NULL, &spec, NULL) == noErr);
    CHECK(spec.name[0] == strlen(leaf));

    /* The spec is optional. */
    CHECK(FSGetCatalogInfo(&ref, 0, NULL, NULL, NULL, NULL) == noErr);

    /* The root directory: the volume name under fsRtParID, as the volume reports it. */
    {
        FSRef root;
        HFSUniStr255 volName;
        FSVolumeRefNum vol;
        FSSpec rootSpec;
        unsigned i;

        memset(&rootSpec, 0xaa, sizeof(rootSpec));
        CHECK(FSPathMakeRef((const UInt8 *)"/", &root, NULL) == noErr);
        CHECK(FSGetCatalogInfo(&root, 0, NULL, NULL, &rootSpec, NULL)
              == noErr);
        CHECK(FSGetVolumeInfo(kFSInvalidVolumeRefNum, 1, &vol,
                              kFSVolInfoNone, NULL, &volName, NULL) == noErr);
        CHECK(rootSpec.vRefNum == vol);
        CHECK(rootSpec.parID == fsRtParID);
        CHECK(rootSpec.name[0] == volName.length);
        for (i = 0; i < volName.length && i < rootSpec.name[0]; i++)
            CHECK(rootSpec.name[1 + i] == (unsigned char)volName.unicode[i]);
    }

    unlink(path);
    printf("CARBONCORE_FSSPEC_CATALOG %s\n", failures ? "FAIL" : "PASS");
    return failures != 0;
}
