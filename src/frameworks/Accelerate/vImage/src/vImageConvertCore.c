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

// The pixel format conversion behind vImageConvert_AnyToAny: unpacks the pixels of a CGImage-style format to floating point
// colour, alpha and (if the colour spaces differ) converts through linear light, then flattens, premultiplies and packs them
// in the destination format. It has no CoreGraphics dependency; vImageConverter.c describes the formats and colour spaces
// to it. The colour spaces are the named ones, converted with their published primaries and transfer curves.
#include "vImageInternal.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// ---- format description

int VIFormatCheck(const VIFormat *f) {
    uint32_t bpc = f->bpc;
    bool gray = VIFormatIsGray(f);
    if (f->space == VISpaceInvalid)
        return -1;
    if (bpc != 1 && bpc != 2 && bpc != 4 && bpc != 5 && bpc != 8 && bpc != 16 && bpc != 32)
        return -1;
    if (f->alphaInfo > VIAlphaNoneSkipFirst)    // VIAlphaOnly (and anything above) is not a valid format
        return -1;
    if (f->isFloat && bpc != 16 && bpc != 32)
        return -1;
    if (bpc == 32 && !f->isFloat)
        return -1;
    uint32_t colors = gray ? 1 : 3;
    uint32_t channels = colors + (f->alphaInfo != VIAlphaNone ? 1 : 0);
    if (bpc < 8) {
        if (bpc == 5) {
            if (gray || f->bpp != 16)
                return -1;
        } else {
            // 1, 2 and 4 bits: gray, with an alpha channel of the same size at most
            if (!gray || (f->bpp != bpc && f->bpp != 2 * bpc) || (f->bpp == bpc && f->alphaInfo != VIAlphaNone) || (f->bpp == 2 * bpc && f->alphaInfo == VIAlphaNone))
                return -1;
        }
    } else {
        if (f->bpp % 8 != 0 || f->bpp < channels * bpc)
            return -1;
    }
    if (f->order == VIOrder16Little || f->order == VIOrder16Big) {
        if (!(bpc == 16 || f->bpp == 16))
            return -1;
    } else if (f->order == VIOrder32Little || f->order == VIOrder32Big) {
        if (!(bpc == 32 || f->bpp == 32))
            return -1;
    } else if (f->order != VIOrderDefault) {
        return -1;
    }
    return 0;
}

// ---- colour spaces

typedef struct { double a[9]; } Mat3;

static Mat3 matMul(const Mat3 *x, const Mat3 *y) {
    Mat3 r;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            r.a[i * 3 + j] = x->a[i * 3] * y->a[j] + x->a[i * 3 + 1] * y->a[3 + j] + x->a[i * 3 + 2] * y->a[6 + j];
    return r;
}

static Mat3 matInverse(const Mat3 *m) {
    const double *a = m->a;
    double c00 = a[4] * a[8] - a[5] * a[7], c01 = a[5] * a[6] - a[3] * a[8], c02 = a[3] * a[7] - a[4] * a[6];
    double det = a[0] * c00 + a[1] * c01 + a[2] * c02;
    Mat3 r;
    double inv = det != 0 ? 1.0 / det : 0;
    r.a[0] = c00 * inv; r.a[1] = (a[2] * a[7] - a[1] * a[8]) * inv; r.a[2] = (a[1] * a[5] - a[2] * a[4]) * inv;
    r.a[3] = c01 * inv; r.a[4] = (a[0] * a[8] - a[2] * a[6]) * inv; r.a[5] = (a[2] * a[3] - a[0] * a[5]) * inv;
    r.a[6] = c02 * inv; r.a[7] = (a[1] * a[6] - a[0] * a[7]) * inv; r.a[8] = (a[0] * a[4] - a[1] * a[3]) * inv;
    return r;
}

// Linear RGB -> XYZ for a D65 white point from the xy chromaticities of the primaries.
static Mat3 rgbToXYZ(double rx, double ry, double gx, double gy, double bx, double by) {
    const double wx = 0.3127, wy = 0.3290;
    Mat3 p = { { rx / ry, gx / gy, bx / by, 1, 1, 1, (1 - rx - ry) / ry, (1 - gx - gy) / gy, (1 - bx - by) / by } };
    Mat3 pi = matInverse(&p);
    double w[3] = { wx / wy, 1, (1 - wx - wy) / wy };
    double s[3];
    for (int i = 0; i < 3; i++)
        s[i] = pi.a[i * 3] * w[0] + pi.a[i * 3 + 1] * w[1] + pi.a[i * 3 + 2] * w[2];
    Mat3 r;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            r.a[i * 3 + j] = p.a[i * 3 + j] * s[j];
    return r;
}

// F: linear colour in the space = F * linear sRGB.
static Mat3 spaceFromSRGB(VISpace s) {
    static const Mat3 identity = { { 1, 0, 0, 0, 1, 0, 0, 0, 1 } };
    // Generic RGB: as measured through the original's conversion (see the research notes).
    static const Mat3 generic = { { 0.97555, 0.02804, -0.00360, -0.02112, 1.05311, -0.03199, 0.00192, 0.00197, 0.99611 } };
    Mat3 srgb = rgbToXYZ(0.64, 0.33, 0.30, 0.60, 0.15, 0.06);
    switch (s) {
    case VISpaceP3:
    case VISpaceLinearP3: {
        Mat3 p3 = rgbToXYZ(0.68, 0.32, 0.265, 0.69, 0.15, 0.06), inv = matInverse(&p3);
        return matMul(&inv, &srgb);
    }
    case VISpaceAdobeRGB: {
        Mat3 adobe = rgbToXYZ(0.64, 0.33, 0.21, 0.71, 0.15, 0.06), inv = matInverse(&adobe);
        return matMul(&inv, &srgb);
    }
    case VISpaceGenericRGB:
    case VISpaceGenericRGBLinear:
        return generic;
    default:
        return identity;
    }
}

static double srgbDecode(double v) { return v <= 0.04045 ? v / 12.92 : pow((v + 0.055) / 1.055, 2.4); }
static double srgbEncode(double v) { return v <= 0.0031308 ? v * 12.92 : 1.055 * pow(v, 1 / 2.4) - 0.055; }

static double trcDecode(VISpace s, double v) {
    double m = fabs(v);
    double r;
    switch (s) {
    case VISpaceSRGB: case VISpaceP3: case VISpaceGray22: r = srgbDecode(m); break;
    case VISpaceAdobeRGB: r = pow(m, 2.19921875); break;
    case VISpaceGenericRGB: case VISpaceGray18: r = pow(m, 1.8); break;
    default: r = m; break;   // the linear spaces
    }
    return v < 0 ? -r : r;
}

static double trcEncode(VISpace s, double v) {
    double m = fabs(v);
    double r;
    switch (s) {
    case VISpaceSRGB: case VISpaceP3: case VISpaceGray22: r = srgbEncode(m); break;
    case VISpaceAdobeRGB: r = pow(m, 1 / 2.19921875); break;
    case VISpaceGenericRGB: case VISpaceGray18: r = pow(m, 1 / 1.8); break;
    default: r = m; break;
    }
    return v < 0 ? -r : r;
}

// ---- the conversion plan

struct VIColorPlan {
    VIFormat s, d;
    int sColors, dColors;
    int mode;               // 0 same space, 1 RGB to RGB, 2 gray to gray, 3 gray to RGB, 4 RGB to gray
    Mat3 m;                 // linear destination RGB = m * linear source RGB (mode 1)
    double yrow[3];         // luminance of linear source RGB (mode 4)
    bool flatten;
    double bg[3];           // quantised to the destination's precision
};

VIColorPlan *VIPlanCreate(const VIFormat *src, const VIFormat *dst, const double *bg) {
    VIColorPlan *p = calloc(1, sizeof *p);
    if (!p)
        return NULL;
    p->s = *src;
    p->d = *dst;
    p->sColors = VIFormatIsGray(src) ? 1 : 3;
    p->dColors = VIFormatIsGray(dst) ? 1 : 3;
    bool sg = p->sColors == 1, dg = p->dColors == 1;
    // The Y row of the sRGB matrix adapted to the D50 profile connection space: what the original gives gray for sRGB colours.
    static const double ySRGB[3] = { 0.2225045, 0.7168786, 0.0606169 };
    if (src->space == dst->space) {
        p->mode = 0;
    } else if (!sg && !dg) {
        Mat3 fs = spaceFromSRGB(src->space), fd = spaceFromSRGB(dst->space), is = matInverse(&fs);
        p->m = matMul(&fd, &is);
        p->mode = 1;
    } else if (sg && dg) {
        p->mode = 2;
    } else if (sg) {
        p->mode = 3;
    } else {
        Mat3 fs = spaceFromSRGB(src->space), is = matInverse(&fs);
        for (int j = 0; j < 3; j++)
            p->yrow[j] = ySRGB[0] * is.a[j] + ySRGB[1] * is.a[3 + j] + ySRGB[2] * is.a[6 + j];
        p->mode = 4;
    }
    p->flatten = VIFormatHasAlpha(src) && !VIFormatHasAlpha(dst);
    for (int i = 0; i < 3; i++) {
        double b = bg ? bg[i] : 0.0;
        if (dg && i > 0)
            b = 0;
        if (!dst->isFloat) {
            double mx = dst->bpc == 5 ? (i == 1 ? 63.0 : 31.0) : (double)((1ull << dst->bpc) - 1);
            if (dst->bpc == 32)
                mx = 4294967295.0;
            double c = b < 0 ? 0 : (b > 1 ? 1 : b);
            b = floor(c * mx + 0.5) / mx;
        }
        p->bg[i] = b;
    }
    return p;
}

void VIPlanDestroy(VIColorPlan *plan) { free(plan); }

// ---- half precision

static float halfToFloat(uint16_t h) {
    uint32_t sign = (uint32_t)(h & 0x8000) << 16, exp = (h >> 10) & 0x1f, man = h & 0x3ff, u;
    if (exp == 0) {
        if (man == 0) {
            u = sign;
        } else {
            exp = 127 - 15 + 1;
            while (!(man & 0x400)) { man <<= 1; exp--; }
            man &= 0x3ff;
            u = sign | (exp << 23) | (man << 13);
        }
    } else if (exp == 31) {
        u = sign | 0x7f800000 | (man << 13);
    } else {
        u = sign | ((exp - 15 + 127) << 23) | (man << 13);
    }
    float f;
    memcpy(&f, &u, 4);
    return f;
}

static uint16_t floatToHalf(float f) {
    uint32_t u;
    memcpy(&u, &f, 4);
    uint32_t sign = (u >> 16) & 0x8000, exp = (u >> 23) & 0xff, man = u & 0x7fffff;
    if (exp == 255)
        return (uint16_t)(sign | 0x7c00 | (man ? 0x200 : 0));
    int e = (int)exp - 127 + 15;
    if (e >= 31)
        return (uint16_t)(sign | 0x7c00);
    if (e <= 0) {
        if (e < -10)
            return (uint16_t)sign;
        man |= 0x800000;
        int shift = 14 - e;
        uint32_t h = man >> shift, rem = man & ((1u << shift) - 1), halfway = 1u << (shift - 1);
        if (rem > halfway || (rem == halfway && (h & 1)))
            h++;
        return (uint16_t)(sign | h);
    }
    uint32_t h = sign | ((uint32_t)e << 10) | (man >> 13);
    uint32_t rem = man & 0x1fff;
    if (rem > 0x1000 || (rem == 0x1000 && (h & 1)))
        h++;
    return (uint16_t)h;
}

// ---- unpacking and packing one pixel

// channels: up to 4 values in the order of the format in memory (alpha first or last if it has one)
static int channelCount(const VIFormat *f, int colors) { return colors + (f->alphaInfo != VIAlphaNone ? 1 : 0); }

static inline uint32_t load16(const uint8_t *p, bool little) { return little ? (uint32_t)p[0] | ((uint32_t)p[1] << 8) : ((uint32_t)p[0] << 8) | p[1]; }
static inline uint32_t load32(const uint8_t *p, bool little) {
    return little ? (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24)
                  : ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}
static inline void store16(uint8_t *p, uint32_t v, bool little) { if (little) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); } else { p[0] = (uint8_t)(v >> 8); p[1] = (uint8_t)v; } }
static inline void store32(uint8_t *p, uint32_t v, bool little) {
    if (little) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }
    else { p[0] = (uint8_t)(v >> 24); p[1] = (uint8_t)(v >> 16); p[2] = (uint8_t)(v >> 8); p[3] = (uint8_t)v; }
}

// Reads pixel x of a row into color[] (straight, not decoded) and alpha (1 when the format has none or skips it); premultiplied
// colour is divided by alpha.
static void unpackPixel(const VIFormat *f, int colors, const uint8_t *row, size_t x, double *color, double *alpha) {
    int total = channelCount(f, colors);
    double ch[4] = { 0, 0, 0, 0 };
    uint32_t bpc = f->bpc;
    if (bpc == 1 || bpc == 2 || bpc == 4) {
        // 1, 2 and 4 bits per component, gray (with alpha last)
        size_t bit = x * f->bpp;
        uint32_t mask = (1u << bpc) - 1;
        for (int i = 0; i < total; i++) {
            size_t b = bit + (size_t)i * bpc;
            uint32_t v = (row[b / 8] >> (8 - bpc - (b % 8))) & mask;
            ch[i] = (double)v / mask;
        }
    } else if (bpc == 5) {
        const uint8_t *p = row + x * 2;
        uint32_t w = load16(p, f->order == VIOrder16Little);
        // The components are widened to 8 bits first (the original converts these formats through an 8-bit image).
        #define WIDEN(v, mx) (floor((double)(v) * 255.0 / (mx) + 0.5) / 255.0)
        if (f->alphaInfo == VIAlphaNone) {
            ch[0] = WIDEN((w >> 11) & 31, 31); ch[1] = WIDEN((w >> 5) & 63, 63); ch[2] = WIDEN(w & 31, 31);
        } else if (f->alphaInfo == VIAlphaFirst || f->alphaInfo == VIAlphaPremultipliedFirst || f->alphaInfo == VIAlphaNoneSkipFirst) {
            ch[0] = (double)((w >> 15) & 1); ch[1] = WIDEN((w >> 10) & 31, 31); ch[2] = WIDEN((w >> 5) & 31, 31); ch[3] = WIDEN(w & 31, 31);
        } else {
            ch[0] = WIDEN((w >> 11) & 31, 31); ch[1] = WIDEN((w >> 6) & 31, 31); ch[2] = WIDEN((w >> 1) & 31, 31); ch[3] = (double)(w & 1);
        }
        #undef WIDEN
    } else {
        size_t bytes = f->bpp / 8;
        const uint8_t *p = row + x * bytes;
        if (bpc == 8) {
            uint8_t b[4] = { 0, 0, 0, 0 };
            for (int i = 0; i < total; i++)
                b[i] = p[i];
            if ((f->order == VIOrder32Little && bytes == 4) || (f->order == VIOrder16Little && bytes == 2)) {
                uint8_t t[4];
                memcpy(t, b, 4);
                for (int i = 0; i < total; i++)
                    b[i] = t[total - 1 - i];
            }
            for (int i = 0; i < total; i++)
                ch[i] = b[i] / 255.0;
        } else if (bpc == 16) {
            bool little = f->order == VIOrder16Little;
            for (int i = 0; i < total; i++) {
                uint32_t w = load16(p + 2 * i, little);
                ch[i] = f->isFloat ? (double)halfToFloat((uint16_t)w) : w / 65535.0;
            }
        } else {
            bool little = f->order == VIOrder32Little;
            for (int i = 0; i < total; i++) {
                uint32_t w = load32(p + 4 * i, little);
                float v;
                memcpy(&v, &w, 4);
                ch[i] = v;
            }
        }
    }
    bool first = f->alphaInfo == VIAlphaFirst || f->alphaInfo == VIAlphaPremultipliedFirst || f->alphaInfo == VIAlphaNoneSkipFirst;
    int off = (f->alphaInfo != VIAlphaNone && first) ? 1 : 0;
    for (int i = 0; i < colors; i++)
        color[i] = ch[off + i];
    double a = 1.0;
    if (VIFormatHasAlpha(f))
        a = ch[first ? 0 : colors];
    if (f->alphaInfo == VIAlphaPremultipliedFirst || f->alphaInfo == VIAlphaPremultipliedLast) {
        for (int i = 0; i < colors; i++)
            color[i] = a != 0 ? color[i] / a : 0.0;
    }
    *alpha = a;
}

static inline double clamp01(double v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
static inline uint32_t quantise(double v, uint32_t mx) { return (uint32_t)floor(clamp01(v) * mx + 0.5); }

static void packPixel(const VIFormat *f, int colors, uint8_t *row, size_t x, const double *color, double alpha) {
    int total = channelCount(f, colors);
    bool first = f->alphaInfo == VIAlphaFirst || f->alphaInfo == VIAlphaPremultipliedFirst || f->alphaInfo == VIAlphaNoneSkipFirst;
    bool premult = f->alphaInfo == VIAlphaPremultipliedFirst || f->alphaInfo == VIAlphaPremultipliedLast;
    double ch[4];
    int pos = 0;
    double aval = VIFormatHasAlpha(f) ? alpha : 1.0;
    if (f->alphaInfo != VIAlphaNone && first)
        ch[pos++] = aval;
    for (int i = 0; i < colors; i++)
        ch[pos++] = premult ? color[i] * alpha : color[i];
    if (f->alphaInfo != VIAlphaNone && !first)
        ch[pos++] = aval;
    uint32_t bpc = f->bpc;
    if (bpc == 1 || bpc == 2 || bpc == 4) {
        size_t bit = x * f->bpp;
        uint32_t mx = (1u << bpc) - 1;
        for (int i = 0; i < total; i++) {
            size_t b = bit + (size_t)i * bpc;
            int shift = 8 - (int)bpc - (int)(b % 8);
            row[b / 8] = (uint8_t)((row[b / 8] & ~(mx << shift)) | (quantise(ch[i], mx) << shift));
        }
    } else if (bpc == 5) {
        bool little = f->order == VIOrder16Little;
        uint32_t w;
        if (f->alphaInfo == VIAlphaNone) {
            w = (quantise(ch[0], 31) << 11) | (quantise(ch[1], 63) << 5) | quantise(ch[2], 31);
        } else if (first) {
            w = (quantise(ch[0], 1) << 15) | (quantise(ch[1], 31) << 10) | (quantise(ch[2], 31) << 5) | quantise(ch[3], 31);
        } else {
            w = (quantise(ch[0], 31) << 11) | (quantise(ch[1], 31) << 6) | (quantise(ch[2], 31) << 1) | quantise(ch[3], 1);
        }
        store16(row + x * 2, w, little);
    } else {
        size_t bytes = f->bpp / 8;
        uint8_t *p = row + x * bytes;
        if (bpc == 8) {
            uint8_t b[4];
            for (int i = 0; i < total; i++)
                b[i] = (uint8_t)quantise(ch[i], 255);
            if ((f->order == VIOrder32Little && bytes == 4) || (f->order == VIOrder16Little && bytes == 2)) {
                for (int i = 0; i < total; i++)
                    p[i] = b[total - 1 - i];
            } else {
                for (int i = 0; i < total; i++)
                    p[i] = b[i];
            }
        } else if (bpc == 16) {
            bool little = f->order == VIOrder16Little;
            for (int i = 0; i < total; i++)
                store16(p + 2 * i, f->isFloat ? floatToHalf((float)ch[i]) : quantise(ch[i], 65535), little);
        } else {
            bool little = f->order == VIOrder32Little;
            for (int i = 0; i < total; i++) {
                float v = (float)ch[i];
                uint32_t w;
                memcpy(&w, &v, 4);
                store32(p + 4 * i, w, little);
            }
        }
    }
}

// ---- running a plan

static void decodeComponents(const VIFormat *f, int colors, double *c) {
    if (!f->hasDecode)
        return;
    for (int i = 0; i < colors; i++)
        c[i] = f->decode[2 * i] + c[i] * (f->decode[2 * i + 1] - f->decode[2 * i]);
}

static void encodeComponents(const VIFormat *f, int colors, double *c) {
    if (!f->hasDecode)
        return;
    for (int i = 0; i < colors; i++) {
        double span = f->decode[2 * i + 1] - f->decode[2 * i];
        c[i] = span != 0 ? (c[i] - f->decode[2 * i]) / span : 0;
    }
}

void VIPlanRun(const VIColorPlan *p, const void *src, size_t srcRowBytes, void *dst, size_t dstRowBytes, size_t width, size_t height) {
    if (width == 0 || height == 0)
        return;
    // One source row is unpacked before the destination row is written, so that the two may be the same memory.
    double *tmp = malloc(width * 4 * sizeof(double));
    if (!tmp)
        return;
    int sc = p->sColors, dc = p->dColors;
    for (size_t y = 0; y < height; y++) {
        const uint8_t *srow = (const uint8_t *)src + y * srcRowBytes;
        uint8_t *drow = (uint8_t *)dst + y * dstRowBytes;
        for (size_t x = 0; x < width; x++) {
            double c[3] = { 0, 0, 0 }, a;
            unpackPixel(&p->s, sc, srow, x, c, &a);
            decodeComponents(&p->s, sc, c);
            double *o = tmp + x * 4;
            switch (p->mode) {
            case 0:
                o[0] = c[0]; o[1] = c[1]; o[2] = c[2];
                break;
            case 1: {
                double l[3] = { trcDecode(p->s.space, c[0]), trcDecode(p->s.space, c[1]), trcDecode(p->s.space, c[2]) };
                for (int i = 0; i < 3; i++)
                    o[i] = trcEncode(p->d.space, p->m.a[i * 3] * l[0] + p->m.a[i * 3 + 1] * l[1] + p->m.a[i * 3 + 2] * l[2]);
                break;
            }
            case 2:
                o[0] = trcEncode(p->d.space, trcDecode(p->s.space, c[0]));
                break;
            case 3: {
                double l = trcDecode(p->s.space, c[0]);
                o[0] = o[1] = o[2] = trcEncode(p->d.space, l);
                break;
            }
            default: {
                double y2 = p->yrow[0] * trcDecode(p->s.space, c[0]) + p->yrow[1] * trcDecode(p->s.space, c[1]) + p->yrow[2] * trcDecode(p->s.space, c[2]);
                o[0] = trcEncode(p->d.space, y2);
                break;
            }
            }
            if (p->flatten) {
                for (int i = 0; i < dc; i++)
                    o[i] = o[i] * a + p->bg[i] * (1 - a);
                a = 1.0;
            }
            o[3] = a;
        }
        if (p->d.bpc < 5 && p->d.bpc != 5)
            memset(drow, 0, (width * p->d.bpp + 7) / 8);   // the bits that no pixel uses are written as zero
        for (size_t x = 0; x < width; x++) {
            double *o = tmp + x * 4;
            double c[3] = { o[0], o[1], o[2] };
            encodeComponents(&p->d, dc, c);
            packPixel(&p->d, dc, drow, x, c, o[3]);
        }
    }
    free(tmp);
}
