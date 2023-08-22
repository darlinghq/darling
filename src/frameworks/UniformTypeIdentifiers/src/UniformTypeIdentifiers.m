/*
 This file is part of Darling.

 Copyright (C) 2023 Darling Team

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


#include <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


extern void* const UTTagClassFilenameExtension = (void*)0;
extern void* const UTTagClassMIMEType = (void*)0;
extern void* const UTType3DContent = (void*)0;
extern void* const UTTypeAIFF = (void*)0;
extern void* const UTTypeARReferenceObject = (void*)0;
extern void* const UTTypeAVI = (void*)0;
extern void* const UTTypeAliasFile = (void*)0;
extern void* const UTTypeAppleArchive = (void*)0;
extern void* const UTTypeAppleProtectedMPEG4Audio = (void*)0;
extern void* const UTTypeAppleProtectedMPEG4Video = (void*)0;
extern void* const UTTypeAppleScript = (void*)0;
extern void* const UTTypeApplication = (void*)0;
extern void* const UTTypeApplicationBundle = (void*)0;
extern void* const UTTypeApplicationExtension = (void*)0;
extern void* const UTTypeArchive = (void*)0;
extern void* const UTTypeAssemblyLanguageSource = (void*)0;
extern void* const UTTypeAudio = (void*)0;
extern void* const UTTypeAudiovisualContent = (void*)0;
extern void* const UTTypeBMP = (void*)0;
extern void* const UTTypeBZ2 = (void*)0;
extern void* const UTTypeBinaryPropertyList = (void*)0;
extern void* const UTTypeBookmark = (void*)0;
extern void* const UTTypeBundle = (void*)0;
extern void* const UTTypeCHeader = (void*)0;
extern void* const UTTypeCPlusPlusHeader = (void*)0;
extern void* const UTTypeCPlusPlusSource = (void*)0;
extern void* const UTTypeCSource = (void*)0;
extern void* const UTTypeCalendarEvent = (void*)0;
extern void* const UTTypeCommaSeparatedText = (void*)0;
extern void* const UTTypeCompositeContent = (void*)0;
extern void* const UTTypeContact = (void*)0;
extern void* const UTTypeContent = (void*)0;
extern void* const UTTypeData = (void*)0;
extern void* const UTTypeDatabase = (void*)0;
extern void* const UTTypeDelimitedText = (void*)0;
extern void* const UTTypeDirectory = (void*)0;
extern void* const UTTypeDiskImage = (void*)0;
extern void* const UTTypeEPUB = (void*)0;
extern void* const UTTypeEXE = (void*)0;
extern void* const UTTypeEmailMessage = (void*)0;
extern void* const UTTypeExecutable = (void*)0;
extern void* const UTTypeFileURL = (void*)0;
extern void* const UTTypeFlatRTFD = (void*)0;
extern void* const UTTypeFolder = (void*)0;
extern void* const UTTypeFont = (void*)0;
extern void* const UTTypeFramework = (void*)0;
extern void* const UTTypeGIF = (void*)0;
extern void* const UTTypeGZIP = (void*)0;
extern void* const UTTypeHEIC = (void*)0;
extern void* const UTTypeHEIF = (void*)0;
extern void* const UTTypeHTML = (void*)0;
extern void* const UTTypeICNS = (void*)0;
extern void* const UTTypeICO = (void*)0;
extern void* const UTTypeImage = (void*)0;
extern void* const UTTypeInternetLocation = (void*)0;
extern void* const UTTypeInternetShortcut = (void*)0;
extern void* const UTTypeItem = (void*)0;
extern void* const UTTypeJPEG = (void*)0;
extern void* const UTTypeJSON = (void*)0;
extern void* const UTTypeJavaScript = (void*)0;
extern void* const UTTypeLivePhoto = (void*)0;
extern void* const UTTypeLog = (void*)0;
extern void* const UTTypeM3UPlaylist = (void*)0;
extern void* const UTTypeMIDI = (void*)0;
extern void* const UTTypeMP3 = (void*)0;
extern void* const UTTypeMPEG = (void*)0;
extern void* const UTTypeMPEG2TransportStream = (void*)0;
extern void* const UTTypeMPEG2Video = (void*)0;
extern void* const UTTypeMPEG4Audio = (void*)0;
extern void* const UTTypeMPEG4Movie = (void*)0;
extern void* const UTTypeMakefile = (void*)0;
extern void* const UTTypeMessage = (void*)0;
extern void* const UTTypeMountPoint = (void*)0;
extern void* const UTTypeMovie = (void*)0;
extern void* const UTTypeOSAScript = (void*)0;
extern void* const UTTypeOSAScriptBundle = (void*)0;
extern void* const UTTypeObjectiveCPlusPlusSource = (void*)0;
extern void* const UTTypeObjectiveCSource = (void*)0;
extern void* const UTTypePDF = (void*)0;
extern void* const UTTypePHPScript = (void*)0;
extern void* const UTTypePKCS12 = (void*)0;
extern void* const UTTypePNG = (void*)0;
extern void* const UTTypePackage = (void*)0;
extern void* const UTTypePerlScript = (void*)0;
extern void* const UTTypePlainText = (void*)0;
extern void* const UTTypePlaylist = (void*)0;
extern void* const UTTypePluginBundle = (void*)0;
extern void* const UTTypePresentation = (void*)0;
extern void* const UTTypePropertyList = (void*)0;
extern void* const UTTypePythonScript = (void*)0;
extern void* const UTTypeQuickLookGenerator = (void*)0;
extern void* const UTTypeQuickTimeMovie = (void*)0;
extern void* const UTTypeRAWImage = (void*)0;
extern void* const UTTypeRTF = (void*)0;
extern void* const UTTypeRTFD = (void*)0;
extern void* const UTTypeRealityFile = (void*)0;
extern void* const UTTypeResolvable = (void*)0;
extern void* const UTTypeRubyScript = (void*)0;
extern void* const UTTypeSVG = (void*)0;
extern void* const UTTypeSceneKitScene = (void*)0;
extern void* const UTTypeScript = (void*)0;
extern void* const UTTypeShellScript = (void*)0;
extern void* const UTTypeSourceCode = (void*)0;
extern void* const UTTypeSpotlightImporter = (void*)0;
extern void* const UTTypeSpreadsheet = (void*)0;
extern void* const UTTypeSwiftSource = (void*)0;
extern void* const UTTypeSymbolicLink = (void*)0;
extern void* const UTTypeSystemPreferencesPane = (void*)0;
extern void* const UTTypeTIFF = (void*)0;
extern void* const UTTypeTabSeparatedText = (void*)0;
extern void* const UTTypeText = (void*)0;
extern void* const UTTypeToDoItem = (void*)0;
extern void* const UTTypeURL = (void*)0;
extern void* const UTTypeURLBookmarkData = (void*)0;
extern void* const UTTypeUSD = (void*)0;
extern void* const UTTypeUSDZ = (void*)0;
extern void* const UTTypeUTF16ExternalPlainText = (void*)0;
extern void* const UTTypeUTF16PlainText = (void*)0;
extern void* const UTTypeUTF8PlainText = (void*)0;
extern void* const UTTypeUTF8TabSeparatedText = (void*)0;
extern void* const UTTypeUnixExecutable = (void*)0;
extern void* const UTTypeVCard = (void*)0;
extern void* const UTTypeVideo = (void*)0;
extern void* const UTTypeVolume = (void*)0;
extern void* const UTTypeWAV = (void*)0;
extern void* const UTTypeWebArchive = (void*)0;
extern void* const UTTypeWebP = (void*)0;
extern void* const UTTypeX509Certificate = (void*)0;
extern void* const UTTypeXML = (void*)0;
extern void* const UTTypeXMLPropertyList = (void*)0;
extern void* const UTTypeXPCService = (void*)0;
extern void* const UTTypeYAML = (void*)0;
extern void* const UTTypeZIP = (void*)0;
extern void* const _UTTagClassBluetoothVendorProductID = (void*)0;
extern void* const _UTTagClassDeviceModelCode = (void*)0;
extern void* const _UTTagClassHFSTypeCode = (void*)0;
extern void* const _UTTagClassPasteboardType = (void*)0;
extern void* const _UTTypeAppCategory = (void*)0;
extern void* const _UTTypeAppCategoryActionGames = (void*)0;
extern void* const _UTTypeAppCategoryAdventureGames = (void*)0;
extern void* const _UTTypeAppCategoryArcadeGames = (void*)0;
extern void* const _UTTypeAppCategoryBoardGames = (void*)0;
extern void* const _UTTypeAppCategoryBookmarks = (void*)0;
extern void* const _UTTypeAppCategoryBooks = (void*)0;
extern void* const _UTTypeAppCategoryBusiness = (void*)0;
extern void* const _UTTypeAppCategoryCardGames = (void*)0;
extern void* const _UTTypeAppCategoryCasinoGames = (void*)0;
extern void* const _UTTypeAppCategoryDeveloperTools = (void*)0;
extern void* const _UTTypeAppCategoryDiceGames = (void*)0;
extern void* const _UTTypeAppCategoryEducation = (void*)0;
extern void* const _UTTypeAppCategoryEducationalGames = (void*)0;
extern void* const _UTTypeAppCategoryEntertainment = (void*)0;
extern void* const _UTTypeAppCategoryFamilyGames = (void*)0;
extern void* const _UTTypeAppCategoryFinance = (void*)0;
extern void* const _UTTypeAppCategoryFoodAndDrink = (void*)0;
extern void* const _UTTypeAppCategoryGames = (void*)0;
extern void* const _UTTypeAppCategoryGraphicsDesign = (void*)0;
extern void* const _UTTypeAppCategoryHealthcareFitness = (void*)0;
extern void* const _UTTypeAppCategoryKidsGames = (void*)0;
extern void* const _UTTypeAppCategoryLifestyle = (void*)0;
extern void* const _UTTypeAppCategoryMagazinesAndNewspapers = (void*)0;
extern void* const _UTTypeAppCategoryMedical = (void*)0;
extern void* const _UTTypeAppCategoryMusic = (void*)0;
extern void* const _UTTypeAppCategoryMusicGames = (void*)0;
extern void* const _UTTypeAppCategoryNavigation = (void*)0;
extern void* const _UTTypeAppCategoryNews = (void*)0;
extern void* const _UTTypeAppCategoryPhotoAndVideo = (void*)0;
extern void* const _UTTypeAppCategoryPhotography = (void*)0;
extern void* const _UTTypeAppCategoryProductivity = (void*)0;
extern void* const _UTTypeAppCategoryPuzzleGames = (void*)0;
extern void* const _UTTypeAppCategoryRacingGames = (void*)0;
extern void* const _UTTypeAppCategoryReference = (void*)0;
extern void* const _UTTypeAppCategoryRolePlayingGames = (void*)0;
extern void* const _UTTypeAppCategoryShopping = (void*)0;
extern void* const _UTTypeAppCategorySimulationGames = (void*)0;
extern void* const _UTTypeAppCategorySocialNetworking = (void*)0;
extern void* const _UTTypeAppCategorySports = (void*)0;
extern void* const _UTTypeAppCategorySportsGames = (void*)0;
extern void* const _UTTypeAppCategoryStrategyGames = (void*)0;
extern void* const _UTTypeAppCategoryTravel = (void*)0;
extern void* const _UTTypeAppCategoryTriviaGames = (void*)0;
extern void* const _UTTypeAppCategoryUtilities = (void*)0;
extern void* const _UTTypeAppCategoryVideo = (void*)0;
extern void* const _UTTypeAppCategoryWeather = (void*)0;
extern void* const _UTTypeAppCategoryWordGames = (void*)0;
extern void* const _UTTypeAppleDevice = (void*)0;
extern void* const _UTTypeAppleEncryptedArchive = (void*)0;
extern void* const _UTTypeAppleTV = (void*)0;
extern void* const _UTTypeAppleWatch = (void*)0;
extern void* const _UTTypeApplicationsFolder = (void*)0;
extern void* const _UTTypeBlockSpecial = (void*)0;
extern void* const _UTTypeCharacterSpecial = (void*)0;
extern void* const _UTTypeComputer = (void*)0;
extern void* const _UTTypeDataContainer = (void*)0;
extern void* const _UTTypeDevice = (void*)0;
extern void* const _UTTypeDisplay = (void*)0;
extern void* const _UTTypeDropFolder = (void*)0;
extern void* const _UTTypeGenericPC = (void*)0;
extern void* const _UTTypeHEIFStandard = (void*)0;
extern void* const _UTTypeHomePod = (void*)0;
extern void* const _UTTypeLibraryFolder = (void*)0;
extern void* const _UTTypeMac = (void*)0;
extern void* const _UTTypeMacBook = (void*)0;
extern void* const _UTTypeMacBookAir = (void*)0;
extern void* const _UTTypeMacBookPro = (void*)0;
extern void* const _UTTypeMacLaptop = (void*)0;
extern void* const _UTTypeMacMini = (void*)0;
extern void* const _UTTypeMacPro = (void*)0;
extern void* const _UTTypeNamedPipeOrFIFO = (void*)0;
extern void* const _UTTypeNetworkNeighborhood = (void*)0;
extern void* const _UTTypePassBundle = (void*)0;
extern void* const _UTTypePassData = (void*)0;
extern void* const _UTTypePassesData = (void*)0;
extern void* const _UTTypeServersFolder = (void*)0;
extern void* const _UTTypeSocket = (void*)0;
extern void* const _UTTypeSpeaker = (void*)0;
extern void* const _UTTypeiMac = (void*)0;
extern void* const _UTTypeiOSDevice = (void*)0;
extern void* const _UTTypeiOSSimulator = (void*)0;
extern void* const _UTTypeiPad = (void*)0;
extern void* const _UTTypeiPhone = (void*)0;
extern void* const _UTTypeiPodTouch = (void*)0;
extern void* const _ZTSSt11logic_error = (void*)0;
extern void* const _ZTSSt12length_error = (void*)0;
extern void* const _ZTSSt19bad_optional_access = (void*)0;
extern void* const _ZTSSt20bad_array_new_length = (void*)0;
extern void* const _ZTSSt9bad_alloc = (void*)0;
extern void* const _ZTSSt9exception = (void*)0;

void *_UTGetAllCoreTypesConstants(void) {
    if (verbose) puts("STUB: _UTGetAllCoreTypesConstants called");
    return NULL;
}

void *_UTHardwareColorGetCurrentEnclosureColor(void) {
    if (verbose) puts("STUB: _UTHardwareColorGetCurrentEnclosureColor called");
    return NULL;
}

void *_UTHardwareColorGetDebugDescription(void) {
    if (verbose) puts("STUB: _UTHardwareColorGetDebugDescription called");
    return NULL;
}

void *_UTHardwareColorMakeWithIndex(void) {
    if (verbose) puts("STUB: _UTHardwareColorMakeWithIndex called");
    return NULL;
}

void *_UTHardwareColorMakeWithRGBComponents(void) {
    if (verbose) puts("STUB: _UTHardwareColorMakeWithRGBComponents called");
    return NULL;
}

void *_UTHardwareColorsAreEqual(void) {
    if (verbose) puts("STUB: _UTHardwareColorsAreEqual called");
    return NULL;
}

void *_UTIdentifierGetCanonicalRepresentation(void) {
    if (verbose) puts("STUB: _UTIdentifierGetCanonicalRepresentation called");
    return NULL;
}

void *_UTIdentifierGetHashCode(void) {
    if (verbose) puts("STUB: _UTIdentifierGetHashCode called");
    return NULL;
}

void *_UTIdentifiersAreEqual(void) {
    if (verbose) puts("STUB: _UTIdentifiersAreEqual called");
    return NULL;
}

void *_UTPrintModelCodesForCurrentDevice(void) {
    if (verbose) puts("STUB: _UTPrintModelCodesForCurrentDevice called");
    return NULL;
}

void *_UTSetRuntimeIssueCatcher(void) {
    if (verbose) puts("STUB: _UTSetRuntimeIssueCatcher called");
    return NULL;
}

void *_UTTaggedTypeCreate(void) {
    if (verbose) puts("STUB: _UTTaggedTypeCreate called");
    return NULL;
}

void *__UNIFORM_TYPE_IDENTIFIER_WAS_NOT_DECLARED_IN_INFO_PLIST_OF_BUNDLE__(void) {
    if (verbose) puts("STUB: __UNIFORM_TYPE_IDENTIFIER_WAS_NOT_DECLARED_IN_INFO_PLIST_OF_BUNDLE__ called");
    return NULL;
}

void *__UTFindCoreTypesConstantWithIdentifier(void) {
    if (verbose) puts("STUB: __UTFindCoreTypesConstantWithIdentifier called");
    return NULL;
}

void *__UTGetDeclarationStatusFromInfoPlist(void) {
    if (verbose) puts("STUB: __UTGetDeclarationStatusFromInfoPlist called");
    return NULL;
}

