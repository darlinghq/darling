#ifndef AT_AUDIO_FORMAT_H
#define AT_AUDIO_FORMAT_H
#include <CoreAudio/CoreAudioTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef UInt32 AudioFormatPropertyID;
typedef UInt32 AudioPanningMode; enum {
 kPanningMode_SoundField = 3,
 kPanningMode_VectorBasedPanning = 4
};
struct AudioPanningInfo {
 AudioPanningMode mPanningMode;
 UInt32 mCoordinateFlags;
 Float32 mCoordinates[3];
 Float32 mGainScale;
 const AudioChannelLayout * mOutputChannelMap;
};
typedef struct AudioPanningInfo AudioPanningInfo;
typedef UInt32 AudioBalanceFadeType; enum {
 kAudioBalanceFadeType_MaxUnityGain = 0,
 kAudioBalanceFadeType_EqualPower = 1
};
struct AudioBalanceFade
{
 Float32 mLeftRightBalance;
 Float32 mBackFrontFade;
 AudioBalanceFadeType mType;
 const AudioChannelLayout* mChannelLayout;
};
typedef struct AudioBalanceFade AudioBalanceFade;
struct AudioFormatInfo
{
 AudioStreamBasicDescription mASBD;
 const void* mMagicCookie;
 UInt32 mMagicCookieSize;
};
typedef struct AudioFormatInfo AudioFormatInfo;
struct ExtendedAudioFormatInfo
{
 AudioStreamBasicDescription mASBD;
 const void* _Nullable mMagicCookie;
 UInt32 mMagicCookieSize;
 AudioClassDescription mClassDescription;
};
typedef struct ExtendedAudioFormatInfo ExtendedAudioFormatInfo;
struct AudioFormatListItem
{
 AudioStreamBasicDescription mASBD;
 AudioChannelLayoutTag mChannelLayoutTag;
};
typedef struct AudioFormatListItem AudioFormatListItem;
enum
{
 kAudioFormatProperty_FormatInfo = 'fmti',
 kAudioFormatProperty_FormatName = 'fnam',
 kAudioFormatProperty_EncodeFormatIDs = 'acof',
 kAudioFormatProperty_DecodeFormatIDs = 'acif',
 kAudioFormatProperty_FormatList = 'flst',
    kAudioFormatProperty_ASBDFromESDS = 'essd',
    kAudioFormatProperty_ChannelLayoutFromESDS = 'escl',
 kAudioFormatProperty_OutputFormatList = 'ofls',
 kAudioFormatProperty_FirstPlayableFormatFromList = 'fpfl',
 kAudioFormatProperty_FormatIsVBR = 'fvbr',
 kAudioFormatProperty_FormatIsExternallyFramed = 'fexf',
 kAudioFormatProperty_FormatIsEncrypted = 'cryp',
 kAudioFormatProperty_Encoders = 'aven',
 kAudioFormatProperty_Decoders = 'avde',
 kAudioFormatProperty_AvailableEncodeBitRates = 'aebr',
 kAudioFormatProperty_AvailableEncodeSampleRates = 'aesr',
 kAudioFormatProperty_AvailableEncodeChannelLayoutTags = 'aecl',
 kAudioFormatProperty_AvailableEncodeNumberChannels = 'avnc',
 kAudioFormatProperty_ASBDFromMPEGPacket = 'admp',
 kAudioFormatProperty_BitmapForLayoutTag = 'bmtg',
 kAudioFormatProperty_MatrixMixMap = 'mmap',
    kAudioFormatProperty_ChannelMap = 'chmp',
 kAudioFormatProperty_NumberOfChannelsForLayout = 'nchm',
 kAudioFormatProperty_AreChannelLayoutsEquivalent = 'cheq',
    kAudioFormatProperty_ChannelLayoutHash = 'chha',
 kAudioFormatProperty_ValidateChannelLayout = 'vacl',
 kAudioFormatProperty_ChannelLayoutForTag = 'cmpl',
 kAudioFormatProperty_TagForChannelLayout = 'cmpt',
 kAudioFormatProperty_ChannelLayoutName = 'lonm',
 kAudioFormatProperty_ChannelLayoutSimpleName = 'lsnm',
 kAudioFormatProperty_ChannelLayoutForBitmap = 'cmpb',
 kAudioFormatProperty_ChannelName = 'cnam',
 kAudioFormatProperty_ChannelShortName = 'csnm',

 kAudioFormatProperty_TagsForNumberOfChannels = 'tagc',
 kAudioFormatProperty_PanningMatrix = 'panm',
 kAudioFormatProperty_BalanceFade = 'balf',

 kAudioFormatProperty_ID3TagSize = 'id3s',
 kAudioFormatProperty_ID3TagToDictionary = 'id3d'
};
extern OSStatus
AudioFormatGetPropertyInfo( AudioFormatPropertyID inPropertyID,
       UInt32 inSpecifierSize,
       const void * _Nullable inSpecifier,
       UInt32 * outPropertyDataSize);
extern OSStatus
AudioFormatGetProperty( AudioFormatPropertyID inPropertyID,
      UInt32 inSpecifierSize,
      const void * _Nullable inSpecifier,
      UInt32 * _Nullable ioPropertyDataSize,
      void * _Nullable outPropertyData);

enum
{
        kAudioFormatUnspecifiedError = 'what',
        kAudioFormatUnsupportedPropertyError = 'prop',
        kAudioFormatBadPropertySizeError = '!siz',
        kAudioFormatBadSpecifierSizeError = '!spc',
        kAudioFormatUnsupportedDataFormatError = 'fmt?',
        kAudioFormatUnknownFormatError = '!fmt'
};

#ifdef __cplusplus
}
#endif

#endif
