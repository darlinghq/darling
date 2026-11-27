/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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
 * Exercise read-only single-fork movie file references.
 * Uses only synthetic local files and public File Manager APIs.
 * Loading the movie itself belongs to NewMovieFromFile tests.
 */
#include <QuickTime/QuickTime.h>
#include <CarbonCore/Files.h>
#include <CarbonCore/MacErrors.h>
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
    char path[] = "/tmp/qt-movie-files-XXXXXX";
    unsigned char bytes[] = {0,0,0,8,'f','r','e','e'};
    unsigned char observed[sizeof(bytes)];
    int fd = mkstemp(path);
    if (fd < 0)
        return 2;

    if (write(fd, bytes, sizeof(bytes)) != sizeof(bytes)) {
        close(fd);
        unlink(path);
        return 2;
    }
    close(fd);

    FSRef ref;
    FSSpec spec;
    short a = -1, b = -1;
    ByteCount count = 0;

    if (FSPathMakeRef((const UInt8 *)path, &ref, NULL) != noErr ||
        FSGetCatalogInfo(&ref, 0, NULL, NULL, &spec, NULL) != noErr) {
        unlink(path);
        return 2;
    }

    CHECK(OpenMovieFile(&spec, &a, fsRdPerm)
          == movieToolboxUninitialized);
    CHECK(a == -1);
    CHECK(EnterMovies() == noErr);

    CHECK(OpenMovieFile(NULL, &a, fsRdPerm) == paramErr);
    CHECK(OpenMovieFile(&spec, NULL, fsRdPerm) == paramErr);
    CHECK(OpenMovieFile(&spec, &a, fsWrPerm) == unimpErr);

    CHECK(OpenMovieFile(&spec, &a, fsRdPerm) == noErr);
    CHECK(OpenMovieFile(&spec, &b, fsRdPerm) == noErr);
    CHECK(a != b);

    if (a != -1 && b != -1) {
        CHECK(FSReadFork(a, fsFromStart, 0,
                         sizeof(observed), observed, &count)
              == noErr);
        CHECK(count == sizeof(bytes));
        CHECK(memcmp(bytes, observed, sizeof(bytes)) == 0);

        CHECK(CloseMovieFile(a) == noErr);
        CHECK(CloseMovieFile(b) == noErr);
        CHECK(CloseMovieFile(a) != noErr);
    }

    unlink(path);

    CHECK(OpenMovieFile(&spec, &a, fsRdPerm) != noErr);
    ExitMovies();

    printf("QUICKTIME_MOVIE_FILES %s\n",
           failures ? "FAIL" : "PASS");
    return failures != 0;
}
