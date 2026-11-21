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

#ifndef AVHostPath_h
#define AVHostPath_h

#import <Foundation/Foundation.h>
#include <libavformat/avformat.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static inline int AVFormatOpenInputHost(AVFormatContext **ps, const char *url, const AVInputFormat *fmt, AVDictionary **options, char **outHostPath)
{
    if (!url) {
        return AVERROR(EINVAL);
    }

    char candidates[4][PATH_MAX];
    int numCandidates = 0;

    const char *sysroot = "/Volumes/SystemRoot";
    size_t sysrootLen = strlen(sysroot);

    // Candidate 1: If starts with /Volumes/SystemRoot, strip it to access host Linux root
    if (strncmp(url, sysroot, sysrootLen) == 0) {
        const char *rest = url + sysrootLen;
        snprintf(candidates[numCandidates++], PATH_MAX, "%s", (rest[0] == '\0') ? "/" : rest);
    }

    // Candidate 2: If path starts with / and not /Volumes/SystemRoot, try prefixing with DARLING_PREFIX
    const char *prefix = getenv("DARLING_PREFIX");
    if (prefix && prefix[0] != '\0' && url[0] == '/' && strncmp(url, sysroot, sysrootLen) != 0) {
        snprintf(candidates[numCandidates++], PATH_MAX, "%s%s", prefix, url);
    }

    // Candidate 3: As-is
    snprintf(candidates[numCandidates++], PATH_MAX, "%s", url);

    int lastErr = AVERROR(ENOENT);
    for (int i = 0; i < numCandidates; i++) {
        int err = avformat_open_input(ps, candidates[i], fmt, options);
        if (err == 0) {
            if (outHostPath) {
                *outHostPath = strdup(candidates[i]);
            }
            return 0;
        }
        lastErr = err;
    }

    return lastErr;
}

#endif /* AVHostPath_h */
