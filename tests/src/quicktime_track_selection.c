/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Track selection by media type or characteristic, media handler
 * descriptions and sound description extensions. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PCM_NATIVE
#include "qt_mov_probe.h"
#else
#include <QuickTime/QuickTime.h>
#include <CarbonCore/Files.h>
#include <CarbonCore/MacMemory.h>
#include <CarbonCore/MacErrors.h>
#include <unistd.h>
#endif

static unsigned checks, failures;

#define CHECK(x) do { ++checks; if (!(x)) { \
    ++failures; printf("FAIL %d %s\n", __LINE__, #x); \
} } while (0)

#define FOURCC(a, b, c, d) (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | \
                            ((uint32_t)(c) << 8) | (uint32_t)(d))

enum { RATE = 8000, SEED = 11 };

static const unsigned char multi[] = {
    0,0,0,211,109,100,97,116,170,187,204,11,40,69,98,127,156,185,214,243,16,45,74,103,
    132,161,190,220,249,22,51,80,109,138,167,196,225,254,27,56,85,114,143,173,202,231,4,33,
    62,91,120,149,178,207,236,9,38,67,96,126,155,184,213,242,15,44,73,102,131,160,189,218,
    247,20,49,79,108,137,166,195,224,253,26,55,84,113,142,171,200,229,2,32,61,90,119,148,
    177,206,235,8,37,66,95,124,153,182,211,241,14,43,72,101,130,159,188,217,246,19,48,77,
    106,135,164,194,223,252,25,54,83,112,141,170,199,228,1,30,59,88,117,147,176,205,234,7,
    36,65,94,123,152,181,210,239,12,41,70,100,129,158,187,216,245,18,47,76,105,134,163,192,
    221,250,23,53,82,111,140,169,198,227,0,29,58,87,116,145,174,203,232,6,35,64,93,122,
    151,180,209,238,11,40,69,98,127,156,185,215,244,17,46,75,104,133,162,0,0,5,20,109,
    111,111,118,0,0,0,108,109,118,104,100,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,2,88,0,0,2,88,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,222,116,114,97,107,0,
    0,0,92,116,107,104,100,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,1,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,122,109,
    100,105,97,0,0,0,32,109,100,104,100,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,2,88,0,0,2,88,0,0,0,0,0,0,0,46,104,100,108,114,0,0,0,0,109,
    104,108,114,118,105,100,101,100,108,110,103,0,0,0,0,0,0,0,0,13,68,97,114,108,
    105,110,103,32,118,105,100,101,111,0,0,0,36,109,105,110,102,0,0,0,28,115,116,98,
    108,0,0,0,20,115,116,115,122,0,0,0,0,0,0,3,232,0,0,0,1,0,0,1,
    126,116,114,97,107,0,0,0,92,116,107,104,100,0,0,0,7,0,0,0,0,0,0,0,
    0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,26,109,100,105,97,0,0,0,32,109,100,104,100,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,31,64,0,0,0,100,0,0,0,0,0,0,0,46,104,100,108,
    114,0,0,0,0,109,104,108,114,115,111,117,110,100,108,110,103,0,0,0,0,0,0,0,
    0,13,68,97,114,108,105,110,103,32,115,111,117,110,100,0,0,0,196,109,105,110,102,0,
    0,0,36,100,105,110,102,0,0,0,28,100,114,101,102,0,0,0,0,0,0,0,1,0,
    0,0,12,97,108,105,115,0,0,0,1,0,0,0,152,115,116,98,108,0,0,0,52,115,
    116,115,100,0,0,0,0,0,0,0,1,0,0,0,36,116,119,111,115,0,0,0,0,0,
    0,0,1,0,0,0,0,0,0,0,0,0,1,0,16,0,0,0,0,31,64,0,0,0,
    0,0,24,115,116,116,115,0,0,0,0,0,0,0,1,0,0,0,100,0,0,0,1,0,
    0,0,28,115,116,115,99,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,100,0,
    0,0,1,0,0,0,20,115,116,115,122,0,0,0,0,0,0,0,2,0,0,0,100,0,
    0,0,20,115,116,99,111,0,0,0,0,0,0,0,1,0,0,0,11,0,0,1,112,116,
    114,97,107,0,0,0,92,116,107,104,100,0,0,0,6,0,0,0,0,0,0,0,0,0,
    0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,12,109,100,105,97,0,0,0,32,109,100,104,100,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,31,64,0,0,0,100,0,0,0,0,0,0,0,32,104,100,108,114,0,
    0,0,0,109,104,108,114,115,111,117,110,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,196,109,105,110,102,0,0,0,36,100,105,110,102,0,0,0,28,100,114,101,102,0,
    0,0,0,0,0,0,1,0,0,0,12,97,108,105,115,0,0,0,1,0,0,0,152,115,
    116,98,108,0,0,0,52,115,116,115,100,0,0,0,0,0,0,0,1,0,0,0,36,116,
    119,111,115,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,16,0,
    0,0,0,31,64,0,0,0,0,0,24,115,116,116,115,0,0,0,0,0,0,0,1,0,
    0,0,100,0,0,0,1,0,0,0,28,115,116,115,99,0,0,0,0,0,0,0,1,0,
    0,0,1,0,0,0,100,0,0,0,1,0,0,0,20,115,116,115,122,0,0,0,0,0,
    0,0,2,0,0,0,100,0,0,0,20,115,116,99,111,0,0,0,0,0,0,0,1,0,
    0,0,11,0,0,0,212,116,114,97,107,0,0,0,92,116,107,104,100,0,0,0,7,0,
    0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,112,109,100,105,97,0,0,0,32,109,100,104,100,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,2,88,0,0,2,88,0,0,0,0,0,
    0,0,36,104,100,108,114,0,0,0,0,109,104,108,114,116,101,120,116,65,66,67,68,0,
    0,0,0,0,0,0,0,97,98,99,0,0,0,0,36,109,105,110,102,0,0,0,28,115,
    116,98,108,0,0,0,20,115,116,115,122,0,0,0,0,0,0,3,232,0,0,0,1
};

static const unsigned char single[] = {
    0,0,0,211,109,100,97,116,170,187,204,11,40,69,98,127,156,185,214,243,16,45,74,103,
    132,161,190,220,249,22,51,80,109,138,167,196,225,254,27,56,85,114,143,173,202,231,4,33,
    62,91,120,149,178,207,236,9,38,67,96,126,155,184,213,242,15,44,73,102,131,160,189,218,
    247,20,49,79,108,137,166,195,224,253,26,55,84,113,142,171,200,229,2,32,61,90,119,148,
    177,206,235,8,37,66,95,124,153,182,211,241,14,43,72,101,130,159,188,217,246,19,48,77,
    106,135,164,194,223,252,25,54,83,112,141,170,199,228,1,30,59,88,117,147,176,205,234,7,
    36,65,94,123,152,181,210,239,12,41,70,100,129,158,187,216,245,18,47,76,105,134,163,192,
    221,250,23,53,82,111,140,169,198,227,0,29,58,87,116,145,174,203,232,6,35,64,93,122,
    151,180,209,238,11,40,69,98,127,156,185,215,244,17,46,75,104,133,162,0,0,2,192,109,
    111,111,118,0,0,0,108,109,118,104,100,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,2,88,0,0,2,88,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,214,116,114,97,107,0,
    0,0,92,116,107,104,100,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,1,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,114,109,
    100,105,97,0,0,0,32,109,100,104,100,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,2,88,0,0,2,88,0,0,0,0,0,0,0,38,104,100,108,114,0,0,0,0,109,
    104,108,114,118,105,100,101,100,108,110,103,0,0,0,0,0,0,0,0,5,118,105,100,101,
    111,0,0,0,36,109,105,110,102,0,0,0,28,115,116,98,108,0,0,0,20,115,116,115,
    122,0,0,0,0,0,0,3,232,0,0,0,1,0,0,1,118,116,114,97,107,0,0,0,
    92,116,107,104,100,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,18,109,100,105,
    97,0,0,0,32,109,100,104,100,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,
    64,0,0,0,100,0,0,0,0,0,0,0,38,104,100,108,114,0,0,0,0,109,104,108,
    114,115,111,117,110,100,108,110,103,0,0,0,0,0,0,0,0,5,115,111,117,110,100,0,
    0,0,196,109,105,110,102,0,0,0,36,100,105,110,102,0,0,0,28,100,114,101,102,0,
    0,0,0,0,0,0,1,0,0,0,12,97,108,105,115,0,0,0,1,0,0,0,152,115,
    116,98,108,0,0,0,52,115,116,115,100,0,0,0,0,0,0,0,1,0,0,0,36,116,
    119,111,115,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,16,0,
    0,0,0,31,64,0,0,0,0,0,24,115,116,116,115,0,0,0,0,0,0,0,1,0,
    0,0,100,0,0,0,1,0,0,0,28,115,116,115,99,0,0,0,0,0,0,0,1,0,
    0,0,1,0,0,0,100,0,0,0,1,0,0,0,20,115,116,115,122,0,0,0,0,0,
    0,0,2,0,0,0,100,0,0,0,20,115,116,99,111,0,0,0,0,0,0,0,1,0,
    0,0,11
};

static const unsigned char ext_0[] = {
    36,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172
};

static const unsigned char ext_1[] = {
    100,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,28,119,97,118,101,0,0,0,12,102,114,109,97,116,119,111,115,
    0,0,0,8,0,0,0,0,0,0,0,20,99,104,97,110,0,0,0,0,0,0,0,0,
    0,0,0,0
};

static const unsigned char ext_2[] = {
    100,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,28,119,97,118,101,0,0,0,12,102,114,109,97,116,119,111,115,
    0,0,0,8,0,0,0,0,0,0,0,20,99,104,97,110,0,0,0,0,0,0,0,0,
    0,0,0,0
};

static const unsigned char ext_3[] = {
    100,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,28,119,97,118,101,0,0,0,12,102,114,109,97,116,119,111,115,
    0,0,0,8,0,0,0,0,0,0,0,20,99,104,97,110,0,0,0,0,0,0,0,0,
    0,0,0,0
};

static const unsigned char ext_4[] = {
    100,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,2,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,28,119,97,118,101,0,0,0,12,102,114,109,97,116,119,111,115,0,0,0,8,
    0,0,0,0
};

static const unsigned char ext_5[] = {
    84,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,28,119,97,118,101,0,0,0,12,102,114,109,97,116,119,111,115,
    0,0,0,8,0,0,0,0,0,0,0,0
};

static const unsigned char ext_6[] = {
    84,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,28,119,97,118,101,0,0,0,12,102,114,109,97,116,119,111,115,
    0,0,0,8,0,0,0,0,0,0,0,0
};

static const unsigned char ext_7[] = {
    60,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,8,119,97,118,101
};

static const unsigned char ext_8[] = {
    52,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,28,119,97,118,101,0,0,0,12,102,114,109,97,116,119,111,115,
    0,0,0,8,0,0,0,0
};

static const unsigned char ext_9[] = {
    40,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,28,119,97,118,101,0,0,0,12,102,114,109,97,116,119,111,115,
    0,0,0,8,0,0,0,0
};

static const unsigned char ext_10[] = {
    84,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,28,119,97,118,101,0,0,0,12,102,114,109,97,116,119,111,115,
    0,0,0,8,0,0,0,0
};

static const unsigned char ext_11[] = {
    60,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,4,119,97,118,101
};

static const unsigned char ext_12[] = {
    68,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,64,119,97,118,101,0,0,0,0,0,0,0,0
};

static const unsigned char ext_13[] = {
    60,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,119,97,118,101
};

static const unsigned char ext_14[] = {
    64,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,3,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172,0,0,0,28,119,97,118,101,0,0,0,12,
    102,114,109,97,116,119,111,115,0,0,0,8,0,0,0,0
};

static const unsigned char ext_15[] = {
    20,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
    2,0,16,0,0,0,0,0,0,0,68,172
};

static const unsigned char ext_16[] = {
    36,0,0,0,115,111,119,116,0,0,0,0,0,0,1,0,0,0,0,0
};

struct track_expect {
    uint32_t handler, manufacturer;
    int enabled;
    const char *name;
};

static const struct track_expect expected_tracks[] = {
    { 0x76696465, 0x646c6e67, 1, "Darling video" },
    { 0x736f756e, 0x646c6e67, 1, "Darling sound" },
    { 0x736f756e, 0x00000000, 0, "" },
    { 0x74657874, 0x41424344, 1, "" },
};

#define TRACK_COUNT (sizeof(expected_tracks) / sizeof(*expected_tracks))

enum { EXT_OK, EXT_MISSING, EXT_BAD };

struct ext_case {
    const unsigned char *data;
    size_t size;
    uint32_t desc_size;
    int version;
    uint32_t type;
    int expect;
    size_t offset, length;
};

static const struct ext_case ext_cases[] = {
    { ext_0, sizeof(ext_0), 36u, 0, 0x77617665u, EXT_MISSING, 0, 0 }, /* v0_no_extension */
    { ext_1, sizeof(ext_1), 100u, 1, 0x77617665u, EXT_OK, 60, 20 }, /* v1_wave */
    { ext_2, sizeof(ext_2), 100u, 1, 0x6368616eu, EXT_OK, 88, 12 }, /* v1_chan */
    { ext_3, sizeof(ext_3), 100u, 1, 0x65736473u, EXT_MISSING, 0, 0 }, /* v1_absent_type */
    { ext_4, sizeof(ext_4), 100u, 2, 0x77617665u, EXT_OK, 80, 20 }, /* v2_wave */
    { ext_5, sizeof(ext_5), 84u, 1, 0x65736473u, EXT_MISSING, 0, 0 }, /* v1_tail_absent */
    { ext_6, sizeof(ext_6), 84u, 1, 0x77617665u, EXT_OK, 60, 20 }, /* v1_tail_found */
    { ext_7, sizeof(ext_7), 60u, 1, 0x77617665u, EXT_OK, 60, 0 }, /* v1_empty_payload */
    { ext_8, sizeof(ext_8), 52u, 1, 0x77617665u, EXT_MISSING, 0, 0 }, /* v1_after_desc_size */
    { ext_9, sizeof(ext_9), 40u, 1, 0x77617665u, EXT_BAD, 0, 0 }, /* v1_desc_size_below_fixed */
    { ext_10, sizeof(ext_10), 84u, 1, 0x77617665u, EXT_BAD, 0, 0 }, /* v1_desc_size_beyond_handle */
    { ext_11, sizeof(ext_11), 60u, 1, 0x77617665u, EXT_BAD, 0, 0 }, /* v1_atom_size_4 */
    { ext_12, sizeof(ext_12), 68u, 1, 0x77617665u, EXT_BAD, 0, 0 }, /* v1_atom_size_overrun */
    { ext_13, sizeof(ext_13), 60u, 1, 0x77617665u, EXT_BAD, 0, 0 }, /* v1_atom_size_0 */
    { ext_14, sizeof(ext_14), 64u, 3, 0x77617665u, EXT_BAD, 0, 0 }, /* version_3 */
    { ext_15, sizeof(ext_15), 20u, 0, 0x77617665u, EXT_BAD, 0, 0 }, /* v0_desc_size_20 */
    { ext_16, sizeof(ext_16), 36u, 0, 0x77617665u, EXT_BAD, 0, 0 }, /* handle_shorter_than_fixed */
};

#define EXT_COUNT (sizeof(ext_cases) / sizeof(*ext_cases))

#ifdef PCM_NATIVE

static void test_tracks(void)
{
    struct qt_mov_track_info info[8];
    size_t n = 0;

    memset(info, 0xa5, sizeof(info));
    CHECK(qt_mov_list_tracks(multi, sizeof(multi), info, 8, &n)
          == QT_MOV_LIST_OK);
    CHECK(n == TRACK_COUNT);

    for (size_t i = 0; i < n && i < TRACK_COUNT; ++i) {
        const struct track_expect *e = &expected_tracks[i];
        size_t len = strlen(e->name);

        CHECK(info[i].has_media == 1);
        CHECK(info[i].handler == e->handler);
        CHECK(info[i].handler_manufacturer == e->manufacturer);
        CHECK(info[i].enabled == e->enabled);
        CHECK(info[i].handler_name[0] == len);
        CHECK(memcmp(info[i].handler_name + 1, e->name, len) == 0);
    }

    unsigned char pcm[200];
    struct qt_mov_pcm_info pi;

    CHECK(qt_mov_extract_pcm(single, sizeof(single), pcm, sizeof(pcm), &pi)
          == QT_MOV_PCM_OK);
    for (unsigned j = 0; j < sizeof(pcm); ++j)
        CHECK(pcm[j] == ((j * 29 + SEED + (j >> 4)) & 255));
}

static void test_extensions(void)
{
    for (size_t i = 0; i < EXT_COUNT; ++i) {
        const struct ext_case *c = &ext_cases[i];
        size_t off = 0x5a5a, len = 0xa5a5;
        int s = qt_sound_description_find_extension(
            c->data, c->size, c->desc_size, c->version, c->type, &off, &len);

        if (c->expect == EXT_OK) {
            CHECK(s == QT_SOUND_EXTENSION_OK);
            CHECK(off == c->offset && len == c->length);
        } else {
            CHECK(s == (c->expect == EXT_MISSING ?
                        QT_SOUND_EXTENSION_NOT_FOUND :
                        QT_SOUND_EXTENSION_BAD_DESCRIPTION));
            CHECK(off == 0x5a5a && len == 0xa5a5);
        }
    }

    size_t off, len;

    CHECK(qt_sound_description_find_extension(NULL, 36, 36, 0, 0,
          &off, &len) == QT_SOUND_EXTENSION_INVALID_ARGUMENT);
    CHECK(qt_sound_description_find_extension(ext_cases[0].data, 36, 36, 0,
          0, NULL, &len) == QT_SOUND_EXTENSION_INVALID_ARGUMENT);
    CHECK(qt_sound_description_find_extension(ext_cases[0].data, 36, 36, 0,
          0, &off, NULL) == QT_SOUND_EXTENSION_INVALID_ARGUMENT);
    CHECK(qt_sound_description_find_extension(ext_cases[0].data, 36, 36, -1,
          0, &off, &len) == QT_SOUND_EXTENSION_BAD_DESCRIPTION);
}

int main(void)
{
    test_tracks();
    test_extensions();
    printf("QT_UP12_NATIVE checks=%u failures=%u\n", checks, failures);
    return failures != 0;
}

#else

static char sentinel_storage;
#define SENTINEL ((Handle)&sentinel_storage)

static unsigned selection_ok, handler_ok, extension_ok, sequence_ok;

static OSErr import_movie(const unsigned char *data, size_t length,
                          Movie *movie, int file)
{
    if (!file) {
        unsigned char *copy = malloc(length);
        if (!copy)
            return memFullErr;
        memcpy(copy, data, length);

        PointerDataRefRecord record = { copy, (Size)length };
        PointerDataRefPtr p = &record;
        OSErr error = NewMovieFromDataRef(
            movie, 0, NULL, (Handle)&p, PointerDataHandlerSubType);

        memset(copy, 0xa5, length);
        free(copy);
        return error;
    }

    char path[] = "/tmp/qt-up12-XXXXXX";
    int fd = mkstemp(path);

    if (fd < 0)
        return ioErr;

    ssize_t written = write(fd, data, length);
    close(fd);

    FSRef ref;
    short fork = -1;
    OSErr error = ioErr;

    if (written == (ssize_t)length) {
        error = FSPathMakeRef((const UInt8 *)path, &ref, NULL);
        if (error == noErr)
            error = FSOpenFork(&ref, 0, NULL, fsRdPerm, &fork);
        if (error == noErr) {
            error = NewMovieFromFile(movie, fork, NULL, NULL, 0, NULL);
            FSCloseFork(fork);
        }
    }

    unlink(path);
    return error;
}

struct query {
    SInt32 index;
    OSType type;
    SInt32 flags;
    int track; /* 1-based track number, 0 when none matches */
};

static void test_selection(void)
{
    unsigned before = failures;
    Movie m = NULL;

    CHECK(import_movie(multi, sizeof(multi), &m, 0) == noErr);
    CHECK(m != NULL);
    if (!m)
        return;

    CHECK(GetMovieTrackCount(m) == (SInt32)TRACK_COUNT);

    Track t[TRACK_COUNT];
    for (SInt32 i = 0; i < (SInt32)TRACK_COUNT; ++i)
        t[i] = GetMovieIndTrack(m, i + 1);

    /* Tracks: 1 video, 2 sound, 3 sound (disabled), 4 text. */
    const OSType text = FOURCC('t', 'e', 'x', 't');
    const OSType music = FOURCC('m', 'u', 's', 'i');
    const SInt32 type = movieTrackMediaType;
    const SInt32 chr = movieTrackCharacteristic;
    const SInt32 on = movieTrackEnabledOnly;
    const struct query q[] = {
        { 1, SoundMediaType, type, 2 },
        { 2, SoundMediaType, type, 3 },
        { 3, SoundMediaType, type, 0 },
        { 1, SoundMediaType, type | on, 2 },
        { 2, SoundMediaType, type | on, 0 },
        { 1, VideoMediaType, type, 1 },
        { 2, VideoMediaType, type, 0 },
        { 1, text, type, 4 },
        { 1, text, type | on, 4 },
        { 1, music, type, 0 },
        { 1, AudioMediaCharacteristic, chr, 2 },
        { 2, AudioMediaCharacteristic, chr, 3 },
        { 2, AudioMediaCharacteristic, chr | on, 0 },
        { 1, VisualMediaCharacteristic, chr, 1 },
        { 2, VisualMediaCharacteristic, chr, 0 },
        { 1, text, chr, 0 },
        { 0, SoundMediaType, type, 0 },
        { -1, SoundMediaType, type, 0 },
    };

    for (size_t i = 0; i < sizeof(q) / sizeof(*q); ++i) {
        Track r = GetMovieIndTrackType(m, q[i].index, q[i].type,
                                       q[i].flags);
        if (q[i].track) {
            CHECK(r == t[q[i].track - 1]);
            CHECK(GetMoviesError() == noErr);
        } else {
            CHECK(r == NULL);
            CHECK(GetMoviesError() == badTrackIndex);
        }
    }

    const SInt32 bad_flags[] = { 0, on, type | chr, type | chr | on };

    for (size_t i = 0; i < sizeof(bad_flags) / sizeof(*bad_flags); ++i) {
        CHECK(GetMovieIndTrackType(m, 1, SoundMediaType, bad_flags[i])
              == NULL);
        CHECK(GetMoviesError() == paramErr);
    }

    Movie bogus = (Movie)&t[0];

    CHECK(GetMovieIndTrackType(bogus, 1, SoundMediaType, type) == NULL);
    CHECK(GetMoviesError() == invalidMovie);

    DisposeMovie(m);
    CHECK(GetMovieIndTrackType(m, 1, SoundMediaType, type) == NULL);
    CHECK(GetMoviesError() == invalidMovie);

    if (failures == before)
        ++selection_ok;
}

static void test_handlers(void)
{
    unsigned before = failures;
    Movie m = NULL;

    CHECK(import_movie(multi, sizeof(multi), &m, 0) == noErr);
    if (!m)
        return;

    for (SInt32 i = 0; i < (SInt32)TRACK_COUNT; ++i) {
        const struct track_expect *e = &expected_tracks[i];
        Media media = GetTrackMedia(GetMovieIndTrack(m, i + 1));
        OSType type = 0x5a5a5a5a, manufacturer = 0x5a5a5a5a;
        Str255 name;
        size_t len = strlen(e->name);

        memset(name, 0xa5, sizeof(name));
        GetMediaHandlerDescription(media, &type, name, &manufacturer);
        CHECK(GetMoviesError() == noErr);
        CHECK(type == e->handler);
        CHECK(manufacturer == e->manufacturer);
        CHECK(name[0] == len);
        CHECK(memcmp(name + 1, e->name, len) == 0);

        type = 0;
        GetMediaHandlerDescription(media, &type, NULL, NULL);
        CHECK(GetMoviesError() == noErr);
        CHECK(type == e->handler);
    }

    OSType type = 0x5a5a5a5a, manufacturer = 0x5a5a5a5a;
    Str255 name;
    Media bogus = (Media)&type;

    name[0] = 7;
    GetMediaHandlerDescription(bogus, &type, name, &manufacturer);
    CHECK(GetMoviesError() == invalidMedia);
    CHECK(type == 0x5a5a5a5a && manufacturer == 0x5a5a5a5a);
    CHECK(name[0] == 7);

    Media media = GetTrackMedia(GetMovieIndTrack(m, 1));
    DisposeMovie(m);
    GetMediaHandlerDescription(media, &type, name, &manufacturer);
    CHECK(GetMoviesError() == invalidMedia);
    CHECK(type == 0x5a5a5a5a);

    if (failures == before)
        ++handler_ok;
}

static void test_extensions(void)
{
    unsigned before = failures;

    for (size_t i = 0; i < EXT_COUNT; ++i) {
        const struct ext_case *c = &ext_cases[i];
        Handle h = NULL;

        CHECK(PtrToHand(c->data, &h, (long)c->size) == noErr);
        if (!h)
            continue;

        Handle ext = SENTINEL;
        OSErr err = GetSoundDescriptionExtension(
            (SoundDescriptionHandle)h, &ext, c->type);

        CHECK(GetMoviesError() == err);
        if (c->expect == EXT_OK) {
            CHECK(err == noErr);
            CHECK(ext != SENTINEL && ext != NULL);
            if (err == noErr && ext && ext != SENTINEL) {
                CHECK(GetHandleSize(ext) == (long)c->length);
                if (c->length && GetHandleSize(ext) == (long)c->length)
                    CHECK(memcmp(*ext, c->data + c->offset, c->length) == 0);
                DisposeHandle(ext);
            }
        } else {
            CHECK(err == (c->expect == EXT_MISSING ?
                          cannotFindAtomErr : paramErr));
            CHECK(ext == SENTINEL);
        }

        CHECK(GetHandleSize(h) == (long)c->size);
        CHECK(memcmp(*h, c->data, c->size) == 0);
        DisposeHandle(h);
    }

    Handle ext = SENTINEL;
    Handle h = NULL;

    CHECK(GetSoundDescriptionExtension(NULL, &ext, 0x77617665) == paramErr);
    CHECK(ext == SENTINEL);
    CHECK(PtrToHand(ext_cases[1].data, &h, (long)ext_cases[1].size)
          == noErr);
    CHECK(GetSoundDescriptionExtension((SoundDescriptionHandle)h, NULL,
                                       0x77617665) == paramErr);
    DisposeHandle(h);

    if (failures == before)
        ++extension_ok;
}

/* The documented sound-reading sequence: select the first enabled sound
 * track, describe it and read its samples. */
static void test_sequence(int file)
{
    unsigned before = failures;
    Movie m = NULL;

    CHECK(import_movie(single, sizeof(single), &m, file) == noErr);
    if (!m)
        return;

    Track track = GetMovieIndTrackType(m, 1, SoundMediaType,
                                       movieTrackMediaType |
                                       movieTrackEnabledOnly);
    CHECK(track != NULL && track == GetMovieIndTrack(m, 2));

    Media media = track ? GetTrackMedia(track) : NULL;
    CHECK(media != NULL);

    if (media) {
        OSType type = 0;
        Handle desc = NewHandle(0);
        Handle pcm = NewHandle(0);
        Handle ext = SENTINEL;

        GetMediaHandlerDescription(media, &type, NULL, NULL);
        CHECK(type == SoundMediaType);
        CHECK(GetMediaTimeScale(media) == RATE);
        CHECK(GetMediaSampleCount(media) == 100);

        GetMediaSampleDescription(media, 1, (SampleDescriptionHandle)desc);
        CHECK(GetMoviesError() == noErr);
        CHECK(GetHandleSize(desc) == 36);
        CHECK(GetSoundDescriptionExtension((SoundDescriptionHandle)desc,
              &ext, 0x77617665) == cannotFindAtomErr);
        CHECK(ext == SENTINEL);

        SInt32 bytes = -1, count = -1, index = -1;
        TimeValue at = -1, duration = -1;
        SInt16 flags = -1;

        CHECK(GetMediaSample(media, pcm, 0, &bytes, 0, &at, &duration,
                             NULL, &index, 100, &count, &flags) == noErr);
        CHECK(bytes == 200 && count == 100 && at == 0);
        CHECK(GetHandleSize(pcm) == 200);
        if (GetHandleSize(pcm) == 200)
            for (unsigned j = 0; j < 200; ++j)
                CHECK((unsigned char)(*pcm)[j] ==
                      ((j * 29 + SEED + (j >> 4)) & 255));

        DisposeHandle(pcm);
        DisposeHandle(desc);
    }

    CHECK(GetMovieIndTrackType(m, 1, VisualMediaCharacteristic,
                               movieTrackCharacteristic)
          == GetMovieIndTrack(m, 1));
    DisposeMovie(m);

    if (failures == before)
        ++sequence_ok;
}

int main(void)
{
    CHECK(EnterMovies() == noErr);

    test_selection();
    test_handlers();
    test_extensions();
    test_sequence(0);
    test_sequence(1);

    ExitMovies();

    Movie none = NULL;
    CHECK(GetMovieIndTrackType(none, 1, SoundMediaType,
                               movieTrackMediaType) == NULL);
    CHECK(GetMoviesError() == movieToolboxUninitialized);

    printf("QT_UP12_TRACKS checks=%u failures=%u selection=%u/1 "
           "handler=%u/1 extension=%u/1 sequence=%u/2\n",
           checks, failures, selection_ok, handler_ok, extension_ok,
           sequence_ok);
    return failures != 0;
}

#endif
