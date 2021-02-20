/**
 * This file is part of Darling.
 *
 * Copyright (C) 2020 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CS_MDITEM_H_
#define _CS_MDITEM_H_

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// common attributes
extern const CFStringRef kMDItemAttributeChangeDate;
extern const CFStringRef kMDItemAudiences;
extern const CFStringRef kMDItemAuthorAddresses;
extern const CFStringRef kMDItemAuthorEmailAddresses;
extern const CFStringRef kMDItemAuthors;
extern const CFStringRef kMDItemCFBundleIdentifier;
extern const CFStringRef kMDItemCity;
extern const CFStringRef kMDItemComment;
extern const CFStringRef kMDItemContactKeywords;
extern const CFStringRef kMDItemContentCreationDate;
extern const CFStringRef kMDItemContentModificationDate;
extern const CFStringRef kMDItemContentType;
extern const CFStringRef kMDItemContributors;
extern const CFStringRef kMDItemCopyright;
extern const CFStringRef kMDItemCountry;
extern const CFStringRef kMDItemCoverage;
extern const CFStringRef kMDItemCreator;
extern const CFStringRef kMDItemDescription;
extern const CFStringRef kMDItemDueDate;
extern const CFStringRef kMDItemDurationSeconds;
extern const CFStringRef kMDItemEmailAddresses;
extern const CFStringRef kMDItemEncodingApplications;
extern const CFStringRef kMDItemFinderComment;
extern const CFStringRef kMDItemFonts;
extern const CFStringRef kMDItemFSHasCustomIcon;
extern const CFStringRef kMDItemFSIsStationery;
extern const CFStringRef kMDItemHeadline;
extern const CFStringRef kMDItemIdentifier;
extern const CFStringRef kMDItemInformation;
extern const CFStringRef kMDItemInstantMessageAddresses;
extern const CFStringRef kMDItemInstructions;
extern const CFStringRef kMDItemKeywords;
extern const CFStringRef kMDItemKind;
extern const CFStringRef kMDItemLanguages;
extern const CFStringRef kMDItemLastUsedDate;
extern const CFStringRef kMDItemNumberOfPages;
extern const CFStringRef kMDItemOrganizations;
extern const CFStringRef kMDItemPageHeight;
extern const CFStringRef kMDItemPageWidth;
extern const CFStringRef kMDItemParticipants;
extern const CFStringRef kMDItemPhoneNumbers;
extern const CFStringRef kMDItemProjects;
extern const CFStringRef kMDItemPublishers;
extern const CFStringRef kMDItemRecipientAddresses;
extern const CFStringRef kMDItemRecipientEmailAddresses;
extern const CFStringRef kMDItemRecipients;
extern const CFStringRef kMDItemRights;
extern const CFStringRef kMDItemSecurityMethod;
extern const CFStringRef kMDItemStarRating;
extern const CFStringRef kMDItemStateOrProvince;
extern const CFStringRef kMDItemSubject;
extern const CFStringRef kMDItemSupportFileType;
extern const CFStringRef kMDItemTextContent;
extern const CFStringRef kMDItemTheme;
extern const CFStringRef kMDItemTitle;
extern const CFStringRef kMDItemURL;
extern const CFStringRef kMDItemVersion;
extern const CFStringRef kMDItemWhereFroms;

// image attributes
extern const CFStringRef kMDItemAcquisitionMake;
extern const CFStringRef kMDItemAcquisitionModel;
extern const CFStringRef kMDItemAlbum;
extern const CFStringRef kMDItemAltitude;
extern const CFStringRef kMDItemAperture;
extern const CFStringRef kMDItemBitsPerSample;
extern const CFStringRef kMDItemColorSpace;
extern const CFStringRef kMDItemEXIFGPSVersion;
extern const CFStringRef kMDItemEXIFVersion;
extern const CFStringRef kMDItemExposureMode;
extern const CFStringRef kMDItemExposureProgram;
extern const CFStringRef kMDItemExposureTimeSeconds;
extern const CFStringRef kMDItemExposureTimeString;
extern const CFStringRef kMDItemFlashOnOff;
extern const CFStringRef kMDItemFNumber;
extern const CFStringRef kMDItemFocalLength;
extern const CFStringRef kMDItemGPSTrack;
extern const CFStringRef kMDItemHasAlphaChannel;
extern const CFStringRef kMDItemImageDirection;
extern const CFStringRef kMDItemISOSpeed;
extern const CFStringRef kMDItemLatitude;
extern const CFStringRef kMDItemLayerNames;
extern const CFStringRef kMDItemLongitude;
extern const CFStringRef kMDItemMaxAperture;
extern const CFStringRef kMDItemMeteringMode;
extern const CFStringRef kMDItemNamedLocation;
extern const CFStringRef kMDItemOrientation;
extern const CFStringRef kMDItemPixelCount;
extern const CFStringRef kMDItemPixelHeight;
extern const CFStringRef kMDItemPixelWidth;
extern const CFStringRef kMDItemProfileName;
extern const CFStringRef kMDItemRedEyeOnOff;
extern const CFStringRef kMDItemResolutionHeightDPI;
extern const CFStringRef kMDItemResolutionWidthDPI;
extern const CFStringRef kMDItemSpeed;
extern const CFStringRef kMDItemTimestamp;
extern const CFStringRef kMDItemWhiteBalance;

// video attributes
extern const CFStringRef kMDItemAudioBitRate;
extern const CFStringRef kMDItemCodecs;
extern const CFStringRef kMDItemDeliveryType;
extern const CFStringRef kMDItemDirector;
extern const CFStringRef kMDItemGenre;
extern const CFStringRef kMDItemMediaTypes;
extern const CFStringRef kMDItemOriginalFormat;
extern const CFStringRef kMDItemOriginalSource;
extern const CFStringRef kMDItemPerformers;
extern const CFStringRef kMDItemProducer;
extern const CFStringRef kMDItemStreamable;
extern const CFStringRef kMDItemTotalBitRate;
extern const CFStringRef kMDItemVideoBitRate;

// audio attributes
extern const CFStringRef kMDItemAppleLoopDescriptors;
extern const CFStringRef kMDItemAppleLoopsKeyFilterType;
extern const CFStringRef kMDItemAppleLoopsLoopMode;
extern const CFStringRef kMDItemAppleLoopsRootKey;
extern const CFStringRef kMDItemAudioChannelCount;
extern const CFStringRef kMDItemAudioEncodingApplication;
extern const CFStringRef kMDItemAudioSampleRate;
extern const CFStringRef kMDItemAudioTrackNumber;
extern const CFStringRef kMDItemComposer;
extern const CFStringRef kMDItemIsGeneralMIDISequence;
extern const CFStringRef kMDItemKeySignature;
extern const CFStringRef kMDItemLyricist;
extern const CFStringRef kMDItemMusicalGenre;
extern const CFStringRef kMDItemMusicalInstrumentCategory;
extern const CFStringRef kMDItemMusicalInstrumentName;
extern const CFStringRef kMDItemRecordingDate;
extern const CFStringRef kMDItemRecordingYear;
extern const CFStringRef kMDItemTempo;
extern const CFStringRef kMDItemTimeSignature;

// filesystem attributes
extern const CFStringRef kMDItemDisplayName;
extern const CFStringRef kMDItemFSContentChangeDate;
extern const CFStringRef kMDItemFSCreationDate;
extern const CFStringRef kMDItemFSExists;
extern const CFStringRef kMDItemFSInvisible;
extern const CFStringRef kMDItemFSIsExtensionHidden;
extern const CFStringRef kMDItemFSIsReadable;
extern const CFStringRef kMDItemFSIsWriteable;
extern const CFStringRef kMDItemFSLabel;
extern const CFStringRef kMDItemFSName;
extern const CFStringRef kMDItemFSNodeCount;
extern const CFStringRef kMDItemFSOwnerGroupID;
extern const CFStringRef kMDItemFSOwnerUserID;
extern const CFStringRef kMDItemFSSize;
extern const CFStringRef kMDItemPath;

// misc. attributes
extern const CFStringRef kMDItemApplicationCategories;
extern const CFStringRef kMDItemCameraOwner;
extern const CFStringRef kMDItemContentTypeTree;
extern const CFStringRef kMDItemDateAdded;
extern const CFStringRef kMDItemDownloadedDate;
extern const CFStringRef kMDItemEditors;
extern const CFStringRef kMDItemExecutableArchitectures;
extern const CFStringRef kMDItemExecutablePlatform;
extern const CFStringRef kMDItemFocalLength35mm;
extern const CFStringRef kMDItemGPSAreaInformation;
extern const CFStringRef kMDItemGPSDateStamp;
extern const CFStringRef kMDItemGPSDestBearing;
extern const CFStringRef kMDItemGPSDestDistance;
extern const CFStringRef kMDItemGPSDestLatitude;
extern const CFStringRef kMDItemGPSDestLongitude;
extern const CFStringRef kMDItemGPSDifferental;
extern const CFStringRef kMDItemGPSDOP;
extern const CFStringRef kMDItemGPSMapDatum;
extern const CFStringRef kMDItemGPSMeasureMode;
extern const CFStringRef kMDItemGPSProcessingMethod;
extern const CFStringRef kMDItemGPSStatus;
extern const CFStringRef kMDItemHTMLContent;
extern const CFStringRef kMDItemIsApplicationManaged;
extern const CFStringRef kMDItemIsLikelyJunk;
extern const CFStringRef kMDItemLabelIcon;
extern const CFStringRef kMDItemLabelID;
extern const CFStringRef kMDItemLabelKind;
extern const CFStringRef kMDItemLabelUUID;
extern const CFStringRef kMDItemLensModel;

typedef struct __MDItem* MDItemRef;

CFTypeID MDItemGetTypeID();

MDItemRef MDItemCreate(CFAllocatorRef allocator, CFStringRef path);
MDItemRef MDItemCreateWithURL(CFAllocatorRef allocator, CFURLRef url);

CFTypeRef MDItemCopyAttribute(MDItemRef item, CFStringRef name);
CFDictionaryRef MDItemCopyAttributes(MDItemRef item, CFArrayRef names);
CFDictionaryRef MDItemCopyAttributeList(MDItemRef item, ...);
CFArrayRef MDItemCopyAttributeNames(MDItemRef item);

Boolean MDItemSetAttribute(MDItemRef item, CFStringRef name, CFTypeRef value);

#ifdef __cplusplus
};
#endif

#endif // _CS_MDITEM_H_
