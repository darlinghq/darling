/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#include <Foundation/Foundation.h>
#include <QTKit/QTKitDefines.h>

@interface QTMetadataItem : NSObject

@end

QTKIT_EXTERN NSString * const QTCommonMetadataAlbumName;
QTKIT_EXTERN NSString * const QTCommonMetadataArtist;
QTKIT_EXTERN NSString * const QTCommonMetadataArtwork;
QTKIT_EXTERN NSString * const QTCommonMetadataAuthor;
QTKIT_EXTERN NSString * const QTCommonMetadataContributor;
QTKIT_EXTERN NSString * const QTCommonMetadataCopyrights;
QTKIT_EXTERN NSString * const QTCommonMetadataCreationDate;
QTKIT_EXTERN NSString * const QTCommonMetadataCreator;
QTKIT_EXTERN NSString * const QTCommonMetadataDescription;
QTKIT_EXTERN NSString * const QTCommonMetadataFormat;
QTKIT_EXTERN NSString * const QTCommonMetadataIdentifier;
QTKIT_EXTERN NSString * const QTCommonMetadataLanguage;
QTKIT_EXTERN NSString * const QTCommonMetadataLastModifiedDate;
QTKIT_EXTERN NSString * const QTCommonMetadataLocation;
QTKIT_EXTERN NSString * const QTCommonMetadataPublisher;
QTKIT_EXTERN NSString * const QTCommonMetadataRelation;
QTKIT_EXTERN NSString * const QTCommonMetadataSoftware;
QTKIT_EXTERN NSString * const QTCommonMetadataSource;
QTKIT_EXTERN NSString * const QTCommonMetadataSubject;
QTKIT_EXTERN NSString * const QTCommonMetadataTitle;
QTKIT_EXTERN NSString * const QTCommonMetadataType;
QTKIT_EXTERN NSString * const QTMetadata3GPUserDataKeyAuthor;
QTKIT_EXTERN NSString * const QTMetadata3GPUserDataKeyCopyright;
QTKIT_EXTERN NSString * const QTMetadata3GPUserDataKeyDescription;
QTKIT_EXTERN NSString * const QTMetadata3GPUserDataKeyGenre;
QTKIT_EXTERN NSString * const QTMetadata3GPUserDataKeyLocation;
QTKIT_EXTERN NSString * const QTMetadata3GPUserDataKeyPerformer;
QTKIT_EXTERN NSString * const QTMetadata3GPUserDataKeyRecordingYear;
QTKIT_EXTERN NSString * const QTMetadata3GPUserDataKeyTitle;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyAlbumName;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyArtist;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyArtwork;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyAuthor;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyComment;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyContributor;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyCopyrights;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyCreationDate;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyCreator;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyDescription;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyFormat;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyIdentifier;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyLanguage;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyLastModifiedDate;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyLocation;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyMake;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyModel;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyPublisher;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyRelation;
QTKIT_EXTERN NSString * const QTMetadataCommonKeySoftware;
QTKIT_EXTERN NSString * const QTMetadataCommonKeySource;
QTKIT_EXTERN NSString * const QTMetadataCommonKeySubject;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyTitle;
QTKIT_EXTERN NSString * const QTMetadataCommonKeyType;
QTKIT_EXTERN NSString * const QTMetadataFormatID3Metadata;
QTKIT_EXTERN NSString * const QTMetadataFormatQuickTimeMetadata;
QTKIT_EXTERN NSString * const QTMetadataFormatQuickTimeUserData;
QTKIT_EXTERN NSString * const QTMetadataFormatiTunesMetadata;
QTKIT_EXTERN NSString * const QTMetadataISOUserDataKeyCopyright;
QTKIT_EXTERN NSString * const QTMetadataKeySpaceCommon;
QTKIT_EXTERN NSString * const QTMetadataKeySpaceID3;
QTKIT_EXTERN NSString * const QTMetadataKeySpaceQuickTimeMetadata;
QTKIT_EXTERN NSString * const QTMetadataKeySpaceQuickTimeUserData;
QTKIT_EXTERN NSString * const QTMetadataKeySpaceiTunes;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyAlbum;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyArranger;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyArtist;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyArtwork;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyAuthor;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyComment;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyComposer;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyCopyright;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyCreationDate;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyCredits;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyDescription;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyDirector;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyDisplayName;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyEncodedBy;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyGenre;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyInformation;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyKeywords;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyLocationISO6709;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyMake;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyModel;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyOriginalArtist;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyPerformer;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyPhonogramRights;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyProducer;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyPublisher;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeySoftware;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyYear;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeMetadataKeyiXML;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyAlbum;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyArranger;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyArtist;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyAuthor;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyChapter;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyComment;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyComposer;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyCopyright;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyCreationDate;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyCredits;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyDescription;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyDirector;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyDisclaimer;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyEncodedBy;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyFullName;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyGenre;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyHostComputer;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyInformation;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyKeywords;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyLocationISO6709;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyMake;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyModel;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyOriginalArtist;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyOriginalFormat;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyOriginalSource;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyPerformers;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyPhonogramRights;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyProducer;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyProduct;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyPublisher;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeySoftware;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeySpecialPlaybackRequirements;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyTrack;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyTrackName;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyURLLink;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyWarning;
QTKIT_EXTERN NSString * const QTMetadataQuickTimeUserDataKeyWriter;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyAccountKind;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyAcknowledgement;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyAlbum;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyAlbumArtist;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyAppleID;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyArranger;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyArtDirector;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyArtist;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyArtistID;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyAuthor;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyBeatsPerMin;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyComposer;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyConductor;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyContentRating;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyCopyright;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyCoverArt;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyCredits;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyDescription;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyDirector;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyDiscCompilation;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyDiscNumber;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyEQ;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyEncodedBy;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyEncodingTool;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyExecProducer;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyGenreID;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyGrouping;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyLinerNotes;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyLyrics;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyOnlineExtras;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyOriginalArtist;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyPerformer;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyPhonogramRights;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyPlaylistID;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyPredefinedGenre;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyProducer;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyPublisher;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyRecordCompany;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyReleaseDate;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeySoloist;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeySongID;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeySongName;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeySoundEngineer;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyThanks;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyTrackNumber;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyTrackSubTitle;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyUserComment;
QTKIT_EXTERN NSString * const QTMetadataiTunesMetadataKeyUserGenre;
