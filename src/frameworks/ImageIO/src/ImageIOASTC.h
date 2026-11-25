// SPDX-License-Identifier: GPL-3.0-or-later
#import <Onyx2D/O2ImageSource.h>

@class O2DataProvider;

// A new image source when the data is a .astc file ImageIO decodes (2D LDR, see ImageIOASTC.m),
// else nil. The caller owns the result and falls back to the Onyx2D decoders on nil.
O2ImageSourceRef ImageIOASTCSourceCreate(O2DataProvider *provider, CFDictionaryRef options);
