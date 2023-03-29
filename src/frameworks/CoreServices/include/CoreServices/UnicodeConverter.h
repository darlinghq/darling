#ifndef _UNICODECONVERTER_H
#define _UNICODECONVERTER_H

#include <CoreServices/MacTypes.h>
#include <CarbonCore/TextCommon.h>
#include <CarbonCore/MixedMode.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef SInt32 UnicodeMapVersion;

struct UnicodeMapping {
    TextEncoding        unicodeEncoding;
    TextEncoding        otherEncoding;
    UnicodeMapVersion   mappingVersion;
};

typedef struct OpaqueTextToUnicodeInfo*    TextToUnicodeInfo;
typedef struct OpaqueUnicodeToTextInfo*    UnicodeToTextInfo;
typedef struct OpaqueUnicodeToTextRunInfo* UnicodeToTextRunInfo;

typedef const TextToUnicodeInfo            ConstTextToUnicodeInfo;
typedef const UnicodeToTextInfo            ConstUnicodeToTextInfo;

typedef struct UnicodeMapping              UnicodeMapping;
typedef UnicodeMapping*                    UnicodeMappingPtr;
typedef const UnicodeMapping*              ConstUnicodeMappingPtr;

extern OSStatus CreateUnicodeToTextInfo(ConstUnicodeMappingPtr iUnicodeMapping, UnicodeToTextInfo *oUnicodeToTextInfo);
extern OSStatus CreateUnicodeToTextInfoByEncoding(TextEncoding iEncoding, UnicodeToTextInfo *oUnicodeToTextInfo);
extern OSStatus CreateUnicodeToTextInfo(ConstUnicodeMappingPtr iUnicodeMapping, UnicodeToTextInfo *oUnicodeToTextInfo);
extern OSStatus CreateUnicodeToTextInfoByEncoding(TextEncoding iEncoding, UnicodeToTextInfo *oUnicodeToTextInfo);

extern OSStatus CreateUnicodeToTextRunInfo(ItemCount iNumberOfMappings,
                                           const UnicodeMapping iUnicodeMappings[],
                                           UnicodeToTextRunInfo *oUnicodeToTextInfo);

extern OSStatus CreateUnicodeToTextRunInfoByEncoding(ItemCount iNumberOfEncodings,
                                                     const TextEncoding iEncodings[],
                                                     UnicodeToTextRunInfo *oUnicodeToTextInfo);

extern OSStatus CreateUnicodeToTextRunInfoByScriptCode(ItemCount iNumberOfScriptCodes,
                                                       const ScriptCode iScripts[],
                                                       UnicodeToTextRunInfo *oUnicodeToTextInfo);

extern OSStatus ChangeTextToUnicodeInfo(TextToUnicodeInfo ioTextToUnicodeInfo, ConstUnicodeMappingPtr iUnicodeMapping);
extern OSStatus ChangeUnicodeToTextInfo(UnicodeToTextInfo ioUnicodeToTextInfo, ConstUnicodeMappingPtr iUnicodeMapping);
extern OSStatus DisposeTextToUnicodeInfo(TextToUnicodeInfo *ioTextToUnicodeInfo);
extern OSStatus DisposeUnicodeToTextInfo(UnicodeToTextInfo *ioUnicodeToTextInfo);
extern OSStatus DisposeUnicodeToTextRunInfo(UnicodeToTextRunInfo *ioUnicodeToTextRunInfo);

extern OSStatus ConvertFromTextToUnicode(TextToUnicodeInfo iTextToUnicodeInfo,
                                         ByteCount iSourceLen,
                                         ConstLogicalAddress iSourceStr,
                                         OptionBits iControlFlags,
                                         ItemCount iOffsetCount,
                                         const ByteOffset iOffsetArray[],
                                         ItemCount *oOffsetCount,
                                         ByteOffset oOffsetArray[],
                                         ByteCount iOutputBufLen,
                                         ByteCount *oSourceRead,
                                         ByteCount *oUnicodeLen,
                                         UniChar oUnicodeStr[]);

extern OSStatus ConvertFromUnicodeToText(UnicodeToTextInfo iUnicodeToTextInfo,
                                         ByteCount iUnicodeLen,
                                         const UniChar iUnicodeStr[],
                                         OptionBits iControlFlags,
                                         ItemCount iOffsetCount,
                                         const ByteOffset iOffsetArray[],
                                         ItemCount *oOffsetCount,
                                         ByteOffset oOffsetArray[],
                                         ByteCount iOutputBufLen,
                                         ByteCount *oInputRead,
                                         ByteCount *oOutputLen,
                                         LogicalAddress oOutputStr);

#ifdef __cplusplus
}
#endif

#endif // _UNICODECONVERTER_H
