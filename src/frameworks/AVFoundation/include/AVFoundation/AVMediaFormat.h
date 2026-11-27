#import <Foundation/Foundation.h>

typedef NSString *AVMetadataKey;
typedef NSString *AVMetadataKeySpace;

extern const AVMetadataKeySpace AVMetadataKeySpaceCommon;

extern const AVMetadataKey AVMetadataCommonKeyAlbumName;
extern const AVMetadataKey AVMetadataCommonKeyArtist;
extern const AVMetadataKey AVMetadataCommonKeyArtwork;
extern const AVMetadataKey AVMetadataCommonKeyDescription;
extern const AVMetadataKey AVMetadataCommonKeyTitle;

extern const AVMetadataKey AVMetadataID3MetadataKeyUserURL;

typedef NSString *AVMediaType;
typedef NSString *AVFileType;

extern const AVMediaType AVMediaTypeAudio;
extern const AVMediaType AVMediaTypeMuxed;
extern const AVMediaType AVMediaTypeVideo;
extern const AVMediaType AVMediaTypeTimecode;
extern const AVMediaType AVMediaTypeMetadata;

extern NSString *const AVMediaCharacteristicVisual;
extern NSString *const AVMediaCharacteristicAudible;

extern const AVFileType AVFileTypeMPEG4;
extern const AVFileType AVFileTypeQuickTimeMovie;
extern AVFileType const AVFileTypeAppleM4A;
