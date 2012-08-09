#ifndef COMMON_PATH_H
#define COMMON_PATH_H

#define DARWIN_MAXPATHLEN 1024
#define DARWIN_MAXNAMLEN 255

// Will try to correct the case of an existing path or existing path segments
// Rationale: HFS+ used on OS X is a case insensitive file system
void translatePathCI(char* path);

// This uses a temporary TLS buffer
char* translatePathCI(const char* path);

#endif
