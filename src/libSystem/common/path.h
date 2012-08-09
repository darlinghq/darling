#ifndef COMMON_PATH_H
#define COMMON_PATH_H

// Will try to correct the case of an existing path or existing path segments
// Rationale: HFS+ used on OS X is a case insensitive file system
void translatePathCI(char* path);

#endif
