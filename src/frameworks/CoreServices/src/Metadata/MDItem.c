#include <CoreServices/MDItem.h>
#include <dispatch/dispatch.h>
#include <CoreFoundation/CFRuntime.h>

struct __MDItem {
	CFRuntimeBase _base;

	// TODO: do stuff with this
};

// common attributes
const CFStringRef kMDItemAttributeChangeDate     = CFSTR("kMDItemAttributeChangeDate");
const CFStringRef kMDItemAudiences               = CFSTR("kMDItemAudiences");
const CFStringRef kMDItemAuthorAddresses         = CFSTR("kMDItemAuthorAddresses");
const CFStringRef kMDItemAuthorEmailAddresses    = CFSTR("kMDItemAuthorEmailAddresses");
const CFStringRef kMDItemAuthors                 = CFSTR("kMDItemAuthors");
const CFStringRef kMDItemCFBundleIdentifier      = CFSTR("kMDItemCFBundleIdentifier");
const CFStringRef kMDItemCity                    = CFSTR("kMDItemCity");
const CFStringRef kMDItemComment                 = CFSTR("kMDItemComment");
const CFStringRef kMDItemContactKeywords         = CFSTR("kMDItemContactKeywords");
const CFStringRef kMDItemContentCreationDate     = CFSTR("kMDItemContentCreationDate");
const CFStringRef kMDItemContentModificationDate = CFSTR("kMDItemContentModificationDate");
const CFStringRef kMDItemContentType             = CFSTR("kMDItemContentType");
const CFStringRef kMDItemContributors            = CFSTR("kMDItemContributors");
const CFStringRef kMDItemCopyright               = CFSTR("kMDItemCopyright");
const CFStringRef kMDItemCountry                 = CFSTR("kMDItemCountry");
const CFStringRef kMDItemCoverage                = CFSTR("kMDItemCoverage");
const CFStringRef kMDItemCreator                 = CFSTR("kMDItemCreator");
const CFStringRef kMDItemDescription             = CFSTR("kMDItemDescription");
const CFStringRef kMDItemDueDate                 = CFSTR("kMDItemDueDate");
const CFStringRef kMDItemDurationSeconds         = CFSTR("kMDItemDurationSeconds");
const CFStringRef kMDItemEmailAddresses          = CFSTR("kMDItemEmailAddresses");
const CFStringRef kMDItemEncodingApplications    = CFSTR("kMDItemEncodingApplications");
const CFStringRef kMDItemFinderComment           = CFSTR("kMDItemFinderComment");
const CFStringRef kMDItemFonts                   = CFSTR("kMDItemFonts");
const CFStringRef kMDItemFSHasCustomIcon         = CFSTR("kMDItemFSHasCustomIcon");
const CFStringRef kMDItemFSIsStationery          = CFSTR("kMDItemFSIsStationery");
const CFStringRef kMDItemHeadline                = CFSTR("kMDItemHeadline");
const CFStringRef kMDItemIdentifier              = CFSTR("kMDItemIdentifier");
const CFStringRef kMDItemInformation             = CFSTR("kMDItemInformation");
const CFStringRef kMDItemInstantMessageAddresses = CFSTR("kMDItemInstantMessageAddresses");
const CFStringRef kMDItemInstructions            = CFSTR("kMDItemInstructions");
const CFStringRef kMDItemKeywords                = CFSTR("kMDItemKeywords");
const CFStringRef kMDItemKind                    = CFSTR("kMDItemKind");
const CFStringRef kMDItemLanguages               = CFSTR("kMDItemLanguages");
const CFStringRef kMDItemLastUsedDate            = CFSTR("kMDItemLastUsedDate");
const CFStringRef kMDItemNumberOfPages           = CFSTR("kMDItemNumberOfPages");
const CFStringRef kMDItemOrganizations           = CFSTR("kMDItemOrganizations");
const CFStringRef kMDItemPageHeight              = CFSTR("kMDItemPageHeight");
const CFStringRef kMDItemPageWidth               = CFSTR("kMDItemPageWidth");
const CFStringRef kMDItemParticipants            = CFSTR("kMDItemParticipants");
const CFStringRef kMDItemPhoneNumbers            = CFSTR("kMDItemPhoneNumbers");
const CFStringRef kMDItemProjects                = CFSTR("kMDItemProjects");
const CFStringRef kMDItemPublishers              = CFSTR("kMDItemPublishers");
const CFStringRef kMDItemRecipientAddresses      = CFSTR("kMDItemRecipientAddresses");
const CFStringRef kMDItemRecipientEmailAddresses = CFSTR("kMDItemRecipientEmailAddresses");
const CFStringRef kMDItemRecipients              = CFSTR("kMDItemRecipients");
const CFStringRef kMDItemRights                  = CFSTR("kMDItemRights");
const CFStringRef kMDItemSecurityMethod          = CFSTR("kMDItemSecurityMethod");
const CFStringRef kMDItemStarRating              = CFSTR("kMDItemStarRating");
const CFStringRef kMDItemStateOrProvince         = CFSTR("kMDItemStateOrProvince");
const CFStringRef kMDItemSubject                 = CFSTR("kMDItemSubject");
const CFStringRef kMDItemSupportFileType         = CFSTR("kMDItemSupportFileType");
const CFStringRef kMDItemTextContent             = CFSTR("kMDItemTextContent");
const CFStringRef kMDItemTheme                   = CFSTR("kMDItemTheme");
const CFStringRef kMDItemTitle                   = CFSTR("kMDItemTitle");
const CFStringRef kMDItemURL                     = CFSTR("kMDItemURL");
const CFStringRef kMDItemVersion                 = CFSTR("kMDItemVersion");
const CFStringRef kMDItemWhereFroms              = CFSTR("kMDItemWhereFroms");

// image attributes
const CFStringRef kMDItemAcquisitionMake     = CFSTR("kMDItemAcquisitionMake");
const CFStringRef kMDItemAcquisitionModel    = CFSTR("kMDItemAcquisitionModel");
const CFStringRef kMDItemAlbum               = CFSTR("kMDItemAlbum");
const CFStringRef kMDItemAltitude            = CFSTR("kMDItemAltitude");
const CFStringRef kMDItemAperture            = CFSTR("kMDItemAperture");
const CFStringRef kMDItemBitsPerSample       = CFSTR("kMDItemBitsPerSample");
const CFStringRef kMDItemColorSpace          = CFSTR("kMDItemColorSpace");
const CFStringRef kMDItemEXIFGPSVersion      = CFSTR("kMDItemEXIFGPSVersion");
const CFStringRef kMDItemEXIFVersion         = CFSTR("kMDItemEXIFVersion");
const CFStringRef kMDItemExposureMode        = CFSTR("kMDItemExposureMode");
const CFStringRef kMDItemExposureProgram     = CFSTR("kMDItemExposureProgram");
const CFStringRef kMDItemExposureTimeSeconds = CFSTR("kMDItemExposureTimeSeconds");
const CFStringRef kMDItemExposureTimeString  = CFSTR("kMDItemExposureTimeString");
const CFStringRef kMDItemFlashOnOff          = CFSTR("kMDItemFlashOnOff");
const CFStringRef kMDItemFNumber             = CFSTR("kMDItemFNumber");
const CFStringRef kMDItemFocalLength         = CFSTR("kMDItemFocalLength");
const CFStringRef kMDItemGPSTrack            = CFSTR("kMDItemGPSTrack");
const CFStringRef kMDItemHasAlphaChannel     = CFSTR("kMDItemHasAlphaChannel");
const CFStringRef kMDItemImageDirection      = CFSTR("kMDItemImageDirection");
const CFStringRef kMDItemISOSpeed            = CFSTR("kMDItemISOSpeed");
const CFStringRef kMDItemLatitude            = CFSTR("kMDItemLatitude");
const CFStringRef kMDItemLayerNames          = CFSTR("kMDItemLayerNames");
const CFStringRef kMDItemLongitude           = CFSTR("kMDItemLongitude");
const CFStringRef kMDItemMaxAperture         = CFSTR("kMDItemMaxAperture");
const CFStringRef kMDItemMeteringMode        = CFSTR("kMDItemMeteringMode");
const CFStringRef kMDItemNamedLocation       = CFSTR("kMDItemNamedLocation");
const CFStringRef kMDItemOrientation         = CFSTR("kMDItemOrientation");
const CFStringRef kMDItemPixelCount          = CFSTR("kMDItemPixelCount");
const CFStringRef kMDItemPixelHeight         = CFSTR("kMDItemPixelHeight");
const CFStringRef kMDItemPixelWidth          = CFSTR("kMDItemPixelWidth");
const CFStringRef kMDItemProfileName         = CFSTR("kMDItemProfileName");
const CFStringRef kMDItemRedEyeOnOff         = CFSTR("kMDItemRedEyeOnOff");
const CFStringRef kMDItemResolutionHeightDPI = CFSTR("kMDItemResolutionHeightDPI");
const CFStringRef kMDItemResolutionWidthDPI  = CFSTR("kMDItemResolutionWidthDPI");
const CFStringRef kMDItemSpeed               = CFSTR("kMDItemSpeed");
const CFStringRef kMDItemTimestamp           = CFSTR("kMDItemTimestamp");
const CFStringRef kMDItemWhiteBalance        = CFSTR("kMDItemWhiteBalance");

// video attributes
const CFStringRef kMDItemAudioBitRate   = CFSTR("kMDItemAudioBitRate");
const CFStringRef kMDItemCodecs         = CFSTR("kMDItemCodecs");
const CFStringRef kMDItemDeliveryType   = CFSTR("kMDItemDeliveryType");
const CFStringRef kMDItemDirector       = CFSTR("kMDItemDirector");
const CFStringRef kMDItemGenre          = CFSTR("kMDItemGenre");
const CFStringRef kMDItemMediaTypes     = CFSTR("kMDItemMediaTypes");
const CFStringRef kMDItemOriginalFormat = CFSTR("kMDItemOriginalFormat");
const CFStringRef kMDItemOriginalSource = CFSTR("kMDItemOriginalSource");
const CFStringRef kMDItemPerformers     = CFSTR("kMDItemPerformers");
const CFStringRef kMDItemProducer       = CFSTR("kMDItemProducer");
const CFStringRef kMDItemStreamable     = CFSTR("kMDItemStreamable");
const CFStringRef kMDItemTotalBitRate   = CFSTR("kMDItemTotalBitRate");
const CFStringRef kMDItemVideoBitRate   = CFSTR("kMDItemVideoBitRate");

// audio attributes
const CFStringRef kMDItemAppleLoopDescriptors      = CFSTR("kMDItemAppleLoopDescriptors");
const CFStringRef kMDItemAppleLoopsKeyFilterType   = CFSTR("kMDItemAppleLoopsKeyFilterType");
const CFStringRef kMDItemAppleLoopsLoopMode        = CFSTR("kMDItemAppleLoopsLoopMode");
const CFStringRef kMDItemAppleLoopsRootKey         = CFSTR("kMDItemAppleLoopsRootKey");
const CFStringRef kMDItemAudioChannelCount         = CFSTR("kMDItemAudioChannelCount");
const CFStringRef kMDItemAudioEncodingApplication  = CFSTR("kMDItemAudioEncodingApplication");
const CFStringRef kMDItemAudioSampleRate           = CFSTR("kMDItemAudioSampleRate");
const CFStringRef kMDItemAudioTrackNumber          = CFSTR("kMDItemAudioTrackNumber");
const CFStringRef kMDItemComposer                  = CFSTR("kMDItemComposer");
const CFStringRef kMDItemIsGeneralMIDISequence     = CFSTR("kMDItemIsGeneralMIDISequence");
const CFStringRef kMDItemKeySignature              = CFSTR("kMDItemKeySignature");
const CFStringRef kMDItemLyricist                  = CFSTR("kMDItemLyricist");
const CFStringRef kMDItemMusicalGenre              = CFSTR("kMDItemMusicalGenre");
const CFStringRef kMDItemMusicalInstrumentCategory = CFSTR("kMDItemMusicalInstrumentCategory");
const CFStringRef kMDItemMusicalInstrumentName     = CFSTR("kMDItemMusicalInstrumentName");
const CFStringRef kMDItemRecordingDate             = CFSTR("kMDItemRecordingDate");
const CFStringRef kMDItemRecordingYear             = CFSTR("kMDItemRecordingYear");
const CFStringRef kMDItemTempo                     = CFSTR("kMDItemTempo");
const CFStringRef kMDItemTimeSignature             = CFSTR("kMDItemTimeSignature");

// filesystem attributes
const CFStringRef kMDItemDisplayName         = CFSTR("kMDItemDisplayName");
const CFStringRef kMDItemFSContentChangeDate = CFSTR("kMDItemFSContentChangeDate");
const CFStringRef kMDItemFSCreationDate      = CFSTR("kMDItemFSCreationDate");
const CFStringRef kMDItemFSExists            = CFSTR("kMDItemFSExists");
const CFStringRef kMDItemFSInvisible         = CFSTR("kMDItemFSInvisible");
const CFStringRef kMDItemFSIsExtensionHidden = CFSTR("kMDItemFSIsExtensionHidden");
const CFStringRef kMDItemFSIsReadable        = CFSTR("kMDItemFSIsReadable");
const CFStringRef kMDItemFSIsWriteable       = CFSTR("kMDItemFSIsWriteable");
const CFStringRef kMDItemFSLabel             = CFSTR("kMDItemFSLabel");
const CFStringRef kMDItemFSName              = CFSTR("kMDItemFSName");
const CFStringRef kMDItemFSNodeCount         = CFSTR("kMDItemFSNodeCount");
const CFStringRef kMDItemFSOwnerGroupID      = CFSTR("kMDItemFSOwnerGroupID");
const CFStringRef kMDItemFSOwnerUserID       = CFSTR("kMDItemFSOwnerUserID");
const CFStringRef kMDItemFSSize              = CFSTR("kMDItemFSSize");
const CFStringRef kMDItemPath                = CFSTR("kMDItemPath");

// misc. attributes
const CFStringRef kMDItemApplicationCategories   = CFSTR("kMDItemApplicationCategories");
const CFStringRef kMDItemCameraOwner             = CFSTR("kMDItemCameraOwner");
const CFStringRef kMDItemContentTypeTree         = CFSTR("kMDItemContentTypeTree");
const CFStringRef kMDItemDateAdded               = CFSTR("kMDItemDateAdded");
const CFStringRef kMDItemDownloadedDate          = CFSTR("kMDItemDownloadedDate");
const CFStringRef kMDItemEditors                 = CFSTR("kMDItemEditors");
const CFStringRef kMDItemExecutableArchitectures = CFSTR("kMDItemExecutableArchitectures");
const CFStringRef kMDItemExecutablePlatform      = CFSTR("kMDItemExecutablePlatform");
const CFStringRef kMDItemFocalLength35mm         = CFSTR("kMDItemFocalLength35mm");
const CFStringRef kMDItemGPSAreaInformation      = CFSTR("kMDItemGPSAreaInformation");
const CFStringRef kMDItemGPSDateStamp            = CFSTR("kMDItemGPSDateStamp");
const CFStringRef kMDItemGPSDestBearing          = CFSTR("kMDItemGPSDestBearing");
const CFStringRef kMDItemGPSDestDistance         = CFSTR("kMDItemGPSDestDistance");
const CFStringRef kMDItemGPSDestLatitude         = CFSTR("kMDItemGPSDestLatitude");
const CFStringRef kMDItemGPSDestLongitude        = CFSTR("kMDItemGPSDestLongitude");
const CFStringRef kMDItemGPSDifferental          = CFSTR("kMDItemGPSDifferental");
const CFStringRef kMDItemGPSDOP                  = CFSTR("kMDItemGPSDOP");
const CFStringRef kMDItemGPSMapDatum             = CFSTR("kMDItemGPSMapDatum");
const CFStringRef kMDItemGPSMeasureMode          = CFSTR("kMDItemGPSMeasureMode");
const CFStringRef kMDItemGPSProcessingMethod     = CFSTR("kMDItemGPSProcessingMethod");
const CFStringRef kMDItemGPSStatus               = CFSTR("kMDItemGPSStatus");
const CFStringRef kMDItemHTMLContent             = CFSTR("kMDItemHTMLContent");
const CFStringRef kMDItemIsApplicationManaged    = CFSTR("kMDItemIsApplicationManaged");
const CFStringRef kMDItemIsLikelyJunk            = CFSTR("kMDItemIsLikelyJunk");
const CFStringRef kMDItemLabelIcon               = CFSTR("kMDItemLabelIcon");
const CFStringRef kMDItemLabelID                 = CFSTR("kMDItemLabelID");
const CFStringRef kMDItemLabelKind               = CFSTR("kMDItemLabelKind");
const CFStringRef kMDItemLabelUUID               = CFSTR("kMDItemLabelUUID");
const CFStringRef kMDItemLensModel               = CFSTR("kMDItemLensModel");

static CFTypeID __kMDItemTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __MDItemClass = {
	_kCFRuntimeScannedObject, // version
	"MDItem",                 // class name
	NULL,                     // init
	NULL,                     // copy
	NULL,                     // deinit
	NULL,                     // equal
	NULL,                     // hash
	NULL,                     // formatting description
	NULL,                     // debug description
};

CFTypeID MDItemGetTypeID() {
	static dispatch_once_t initOnce;
	dispatch_once(&initOnce, ^{
		__kMDItemTypeID = _CFRuntimeRegisterClass(&__MDItemClass);
	});
	return __kMDItemTypeID;
};

static MDItemRef _MDItemBasicCreate(CFAllocatorRef allocator) {
	MDItemRef item = (MDItemRef)_CFRuntimeCreateInstance(allocator, MDItemGetTypeID(), sizeof(struct __MDItem) - sizeof(CFRuntimeBase), NULL);
	if (!item) {
		return NULL;
	}

	// TODO: actually initialize the instance, once we do stuff with it

	return item;
};

MDItemRef MDItemCreate(CFAllocatorRef allocator, CFStringRef path) {
	MDItemRef item = _MDItemBasicCreate(allocator);

	// TODO: do stuff

	return item;
};

MDItemRef MDItemCreateWithURL(CFAllocatorRef allocator, CFURLRef url) {
	MDItemRef item = _MDItemBasicCreate(allocator);

	// TODO: do stuff

	return item;
};

CFTypeRef MDItemCopyAttribute(MDItemRef item, CFStringRef name) {
	// stub
	return NULL;
};

CFDictionaryRef MDItemCopyAttributes(MDItemRef item, CFArrayRef names) {
	// stub
	return NULL;
};

CFDictionaryRef MDItemCopyAttributeList(MDItemRef item, ...) {
	// stub
	return NULL;
};

CFArrayRef MDItemCopyAttributeNames(MDItemRef item) {
	// stub
	return NULL;
};

Boolean MDItemSetAttribute(MDItemRef item, CFStringRef name, CFTypeRef value) {
	// i'm guessing falsy return indicates error? (this is a private function, and the code calling it does no error checking)
	return false;
};
