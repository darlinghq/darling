/*
 This file is part of Darling.

 Copyright (C) 2019-2026 Darling Developers

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

#import <QTKit/QTUtilities.h>
#import <QTKit/QTMedia.h>
#import <darling-stub-forwarding.h>

@implementation QTUtilities

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end

OSType QTOSTypeForString(NSString *string)
{
    // QUIRK: the first four characters, in Mac OS Roman, with the first one in the highest byte; a shorter
    // string is not padded, so its characters fill the low bytes ("abc" is 0x00616263), and the empty
    // string is 0. Black-box observation of macOS 10.14.4 (tier 2). Test: tests/qtkit-time.m "OSType strings".
    if (!string || [string length] == 0) return 0;
    NSData *data = [string dataUsingEncoding:NSMacOSRomanStringEncoding allowLossyConversion:YES];
    if (!data) return 0;
    const uint8_t *bytes = (const uint8_t *)[data bytes];
    NSUInteger len = MIN([data length], (NSUInteger)4);
    OSType result = 0;
    for (NSUInteger i = 0; i < len; i++)
        result = (result << 8) | bytes[i];
    return result;
}

NSString *QTStringForOSType(OSType osType)
{
    char s[5];
    s[0] = (char)((osType >> 24) & 0xFF);
    s[1] = (char)((osType >> 16) & 0xFF);
    s[2] = (char)((osType >> 8) & 0xFF);
    s[3] = (char)(osType & 0xFF);
    s[4] = '\0';
    return [NSString stringWithCString:s encoding:NSMacOSRomanStringEncoding] ?:
           [NSString stringWithCString:s encoding:NSASCIIStringEncoding] ?:
           [NSString stringWithFormat:@"%c%c%c%c", s[0], s[1], s[2], s[3]];
}

NSString *QTMediaTypeStringForOSType(OSType osType)
{
    switch (osType) {
    case 'vide': return QTMediaTypeVideo;
    case 'soun': return QTMediaTypeSound;
    case 'text': return QTMediaTypeText;
    case 'subt': return QTMediaTypeSubtitle;
    case 'clcp': return QTMediaTypeClosedCaption;
    case 'musi': return QTMediaTypeMusic;
    case 'mpeq': return QTMediaTypeMPEG;
    case 'sprm': return QTMediaTypeSprite;
    case 'flsh': return QTMediaTypeFlash;
    case 'q3d ': return QTMediaType3D;
    case 'qtvr': return QTMediaTypeQTVR;
    case 'qtz ': return QTMediaTypeQuartzComposer;
    case 'hint': return QTMediaTypeHint;
    case 'strm': return QTMediaTypeStream;
    case 'twen': return QTMediaTypeTween;
    case 'skin': return QTMediaTypeSkin;
    case 'qtcm': return QTMediaTypeMuxed;
    case 'tmcd': return QTMediaTypeTimeCode;
    default: return QTStringForOSType(osType);
    }
}
