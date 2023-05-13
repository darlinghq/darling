#include <CoreServices/UnicodeConverter.h>
#include <CarbonCore/TextCommon.h>
#include <cstdio>

OSStatus CreateTextToUnicodeInfo(ConstUnicodeMappingPtr iUnicodeMapping, TextToUnicodeInfo *oTextToUnicodeInfo)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

OSStatus CreateTextToUnicodeInfoByEncoding(TextEncoding iEncoding, TextToUnicodeInfo *oTextToUnicodeInfo)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

OSStatus CreateUnicodeToTextInfo(ConstUnicodeMappingPtr iUnicodeMapping, UnicodeToTextInfo *oUnicodeToTextInfo)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    printf("Encoding: %02x, other encoding: %02x, version: %02x\n",
            iUnicodeMapping->unicodeEncoding,
            iUnicodeMapping->otherEncoding,
            iUnicodeMapping->mappingVersion);

    return 0;
}

OSStatus CreateUnicodeToTextInfoByEncoding(TextEncoding iEncoding, UnicodeToTextInfo *oUnicodeToTextInfo)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

OSStatus CreateUnicodeToTextRunInfo(ItemCount iNumberOfMappings,
                                    const UnicodeMapping iUnicodeMappings[],
                                    UnicodeToTextRunInfo * oUnicodeToTextInfo)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

OSStatus CreateUnicodeToTextRunInfoByEncoding(ItemCount iNumberOfEncodings,
                                              const TextEncoding iEncodings[],
                                              UnicodeToTextRunInfo *oUnicodeToTextInfo)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

OSStatus CreateUnicodeToTextRunInfoByScriptCode(ItemCount iNumberOfScriptCodes,
                                                const ScriptCode iScripts[],
                                                UnicodeToTextRunInfo *oUnicodeToTextInfo)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

OSStatus ChangeTextToUnicodeInfo(TextToUnicodeInfo ioTextToUnicodeInfo, ConstUnicodeMappingPtr iUnicodeMapping)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

OSStatus ChangeUnicodeToTextInfo(UnicodeToTextInfo ioUnicodeToTextInfo, ConstUnicodeMappingPtr iUnicodeMapping)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

OSStatus DisposeTextToUnicodeInfo(TextToUnicodeInfo *ioTextToUnicodeInfo)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

OSStatus DisposeUnicodeToTextInfo(UnicodeToTextInfo *ioUnicodeToTextInfo)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

OSStatus DisposeUnicodeToTextRunInfo(UnicodeToTextRunInfo * ioUnicodeToTextRunInfo)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

OSStatus ConvertFromTextToUnicode(TextToUnicodeInfo iTextToUnicodeInfo,
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
                                  UniChar oUnicodeStr[])
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}

OSStatus ConvertFromUnicodeToText(UnicodeToTextInfo iUnicodeToTextInfo,
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
                                  LogicalAddress oOutputStr)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
    return 0;
}
