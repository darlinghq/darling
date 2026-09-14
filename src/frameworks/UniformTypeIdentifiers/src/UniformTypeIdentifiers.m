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


NSString *const UTTagClassFilenameExtension = @"public.filename-extension";
NSString *const UTTagClassMIMEType = @"public.mime-type";

// The UTType constants are defined in UTType.m, next to the built-in type table that fills them at load.
void* const _UTTagClassBluetoothVendorProductID = (void*)0;
void* const _UTTagClassDeviceModelCode = (void*)0;
void* const _UTTagClassHFSTypeCode = (void*)0;
void* const _UTTagClassPasteboardType = (void*)0;
void* const _UTTypeAppCategory = (void*)0;
void* const _UTTypeAppCategoryActionGames = (void*)0;
void* const _UTTypeAppCategoryAdventureGames = (void*)0;
void* const _UTTypeAppCategoryArcadeGames = (void*)0;
void* const _UTTypeAppCategoryBoardGames = (void*)0;
void* const _UTTypeAppCategoryBookmarks = (void*)0;
void* const _UTTypeAppCategoryBooks = (void*)0;
void* const _UTTypeAppCategoryBusiness = (void*)0;
void* const _UTTypeAppCategoryCardGames = (void*)0;
void* const _UTTypeAppCategoryCasinoGames = (void*)0;
void* const _UTTypeAppCategoryDeveloperTools = (void*)0;
void* const _UTTypeAppCategoryDiceGames = (void*)0;
void* const _UTTypeAppCategoryEducation = (void*)0;
void* const _UTTypeAppCategoryEducationalGames = (void*)0;
void* const _UTTypeAppCategoryEntertainment = (void*)0;
void* const _UTTypeAppCategoryFamilyGames = (void*)0;
void* const _UTTypeAppCategoryFinance = (void*)0;
void* const _UTTypeAppCategoryFoodAndDrink = (void*)0;
void* const _UTTypeAppCategoryGames = (void*)0;
void* const _UTTypeAppCategoryGraphicsDesign = (void*)0;
void* const _UTTypeAppCategoryHealthcareFitness = (void*)0;
void* const _UTTypeAppCategoryKidsGames = (void*)0;
void* const _UTTypeAppCategoryLifestyle = (void*)0;
void* const _UTTypeAppCategoryMagazinesAndNewspapers = (void*)0;
void* const _UTTypeAppCategoryMedical = (void*)0;
void* const _UTTypeAppCategoryMusic = (void*)0;
void* const _UTTypeAppCategoryMusicGames = (void*)0;
void* const _UTTypeAppCategoryNavigation = (void*)0;
void* const _UTTypeAppCategoryNews = (void*)0;
void* const _UTTypeAppCategoryPhotoAndVideo = (void*)0;
void* const _UTTypeAppCategoryPhotography = (void*)0;
void* const _UTTypeAppCategoryProductivity = (void*)0;
void* const _UTTypeAppCategoryPuzzleGames = (void*)0;
void* const _UTTypeAppCategoryRacingGames = (void*)0;
void* const _UTTypeAppCategoryReference = (void*)0;
void* const _UTTypeAppCategoryRolePlayingGames = (void*)0;
void* const _UTTypeAppCategoryShopping = (void*)0;
void* const _UTTypeAppCategorySimulationGames = (void*)0;
void* const _UTTypeAppCategorySocialNetworking = (void*)0;
void* const _UTTypeAppCategorySports = (void*)0;
void* const _UTTypeAppCategorySportsGames = (void*)0;
void* const _UTTypeAppCategoryStrategyGames = (void*)0;
void* const _UTTypeAppCategoryTravel = (void*)0;
void* const _UTTypeAppCategoryTriviaGames = (void*)0;
void* const _UTTypeAppCategoryUtilities = (void*)0;
void* const _UTTypeAppCategoryVideo = (void*)0;
void* const _UTTypeAppCategoryWeather = (void*)0;
void* const _UTTypeAppCategoryWordGames = (void*)0;
void* const _UTTypeAppleDevice = (void*)0;
void* const _UTTypeAppleEncryptedArchive = (void*)0;
void* const _UTTypeAppleTV = (void*)0;
void* const _UTTypeAppleWatch = (void*)0;
void* const _UTTypeApplicationsFolder = (void*)0;
void* const _UTTypeBlockSpecial = (void*)0;
void* const _UTTypeCharacterSpecial = (void*)0;
void* const _UTTypeComputer = (void*)0;
void* const _UTTypeDataContainer = (void*)0;
void* const _UTTypeDevice = (void*)0;
void* const _UTTypeDisplay = (void*)0;
void* const _UTTypeDropFolder = (void*)0;
void* const _UTTypeGenericPC = (void*)0;
void* const _UTTypeHEIFStandard = (void*)0;
void* const _UTTypeHomePod = (void*)0;
void* const _UTTypeLibraryFolder = (void*)0;
void* const _UTTypeMac = (void*)0;
void* const _UTTypeMacBook = (void*)0;
void* const _UTTypeMacBookAir = (void*)0;
void* const _UTTypeMacBookPro = (void*)0;
void* const _UTTypeMacLaptop = (void*)0;
void* const _UTTypeMacMini = (void*)0;
void* const _UTTypeMacPro = (void*)0;
void* const _UTTypeNamedPipeOrFIFO = (void*)0;
void* const _UTTypeNetworkNeighborhood = (void*)0;
void* const _UTTypePassBundle = (void*)0;
void* const _UTTypePassData = (void*)0;
void* const _UTTypePassesData = (void*)0;
void* const _UTTypeServersFolder = (void*)0;
void* const _UTTypeSocket = (void*)0;
void* const _UTTypeSpeaker = (void*)0;
void* const _UTTypeiMac = (void*)0;
void* const _UTTypeiOSDevice = (void*)0;
void* const _UTTypeiOSSimulator = (void*)0;
void* const _UTTypeiPad = (void*)0;
void* const _UTTypeiPhone = (void*)0;
void* const _UTTypeiPodTouch = (void*)0;
void* const _ZTSSt11logic_error = (void*)0;
void* const _ZTSSt12length_error = (void*)0;
void* const _ZTSSt19bad_optional_access = (void*)0;
void* const _ZTSSt20bad_array_new_length = (void*)0;
void* const _ZTSSt9bad_alloc = (void*)0;
void* const _ZTSSt9exception = (void*)0;

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

