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

#ifndef _UNIFORMTYPEIDENTIFIERS_H_
#define _UNIFORMTYPEIDENTIFIERS_H_

#import <Foundation/Foundation.h>

#import <UniformTypeIdentifiers/NSItemProvider+UTType.h>
#import <UniformTypeIdentifiers/NSString+UTAdditions.h>
#import <UniformTypeIdentifiers/NSURL+UTAdditions.h>
#import <UniformTypeIdentifiers/UTType.h>
#import <UniformTypeIdentifiers/UTCoreTypes.h>
#import <UniformTypeIdentifiers/_UTConstantType.h>
#import <UniformTypeIdentifiers/_UTCoreType+UTRefCounting.h>
#import <UniformTypeIdentifiers/_UTCoreType.h>
#import <UniformTypeIdentifiers/_UTRuntimeConstantType.h>
#import <UniformTypeIdentifiers/_UTTaggedType.h>


void *_UTGetAllCoreTypesConstants(void);
void *_UTHardwareColorGetCurrentEnclosureColor(void);
void *_UTHardwareColorGetDebugDescription(void);
void *_UTHardwareColorMakeWithIndex(void);
void *_UTHardwareColorMakeWithRGBComponents(void);
void *_UTHardwareColorsAreEqual(void);
void *_UTIdentifierGetCanonicalRepresentation(void);
void *_UTIdentifierGetHashCode(void);
void *_UTIdentifiersAreEqual(void);
void *_UTPrintModelCodesForCurrentDevice(void);
void *_UTSetRuntimeIssueCatcher(void);
void *_UTTaggedTypeCreate(void);
void *__UNIFORM_TYPE_IDENTIFIER_WAS_NOT_DECLARED_IN_INFO_PLIST_OF_BUNDLE__(void);
void *__UTFindCoreTypesConstantWithIdentifier(void);
void *__UTGetDeclarationStatusFromInfoPlist(void);


extern UTType *const UTType3DContent;
extern UTType *const UTTypeAIFF;
extern UTType *const UTTypeARReferenceObject;
extern UTType *const UTTypeAVI;
extern UTType *const UTTypeAliasFile;
extern UTType *const UTTypeAppleArchive;
extern UTType *const UTTypeAppleProtectedMPEG4Audio;
extern UTType *const UTTypeAppleProtectedMPEG4Video;
extern UTType *const UTTypeAppleScript;
extern UTType *const UTTypeApplication;
extern UTType *const UTTypeApplicationBundle;
extern UTType *const UTTypeApplicationExtension;
extern UTType *const UTTypeArchive;
extern UTType *const UTTypeAssemblyLanguageSource;
extern UTType *const UTTypeAudio;
extern UTType *const UTTypeAudiovisualContent;
extern UTType *const UTTypeBMP;
extern UTType *const UTTypeBZ2;
extern UTType *const UTTypeBinaryPropertyList;
extern UTType *const UTTypeBookmark;
extern UTType *const UTTypeBundle;
extern UTType *const UTTypeCHeader;
extern UTType *const UTTypeCPlusPlusHeader;
extern UTType *const UTTypeCPlusPlusSource;
extern UTType *const UTTypeCSource;
extern UTType *const UTTypeCalendarEvent;
extern UTType *const UTTypeCommaSeparatedText;
extern UTType *const UTTypeCompositeContent;
extern UTType *const UTTypeContact;
extern UTType *const UTTypeContent;
extern UTType *const UTTypeData;
extern UTType *const UTTypeDatabase;
extern UTType *const UTTypeDelimitedText;
extern UTType *const UTTypeDirectory;
extern UTType *const UTTypeDiskImage;
extern UTType *const UTTypeEPUB;
extern UTType *const UTTypeEXE;
extern UTType *const UTTypeEmailMessage;
extern UTType *const UTTypeExecutable;
extern UTType *const UTTypeFileURL;
extern UTType *const UTTypeFlatRTFD;
extern UTType *const UTTypeFolder;
extern UTType *const UTTypeFont;
extern UTType *const UTTypeFramework;
extern UTType *const UTTypeGIF;
extern UTType *const UTTypeGZIP;
extern UTType *const UTTypeHEIC;
extern UTType *const UTTypeHEIF;
extern UTType *const UTTypeHTML;
extern UTType *const UTTypeICNS;
extern UTType *const UTTypeICO;
extern UTType *const UTTypeImage;
extern UTType *const UTTypeInternetLocation;
extern UTType *const UTTypeInternetShortcut;
extern UTType *const UTTypeItem;
extern UTType *const UTTypeJPEG;
extern UTType *const UTTypeJSON;
extern UTType *const UTTypeJavaScript;
extern UTType *const UTTypeLivePhoto;
extern UTType *const UTTypeLog;
extern UTType *const UTTypeM3UPlaylist;
extern UTType *const UTTypeMIDI;
extern UTType *const UTTypeMP3;
extern UTType *const UTTypeMPEG;
extern UTType *const UTTypeMPEG2TransportStream;
extern UTType *const UTTypeMPEG2Video;
extern UTType *const UTTypeMPEG4Audio;
extern UTType *const UTTypeMPEG4Movie;
extern UTType *const UTTypeMakefile;
extern UTType *const UTTypeMessage;
extern UTType *const UTTypeMountPoint;
extern UTType *const UTTypeMovie;
extern UTType *const UTTypeOSAScript;
extern UTType *const UTTypeOSAScriptBundle;
extern UTType *const UTTypeObjectiveCPlusPlusSource;
extern UTType *const UTTypeObjectiveCSource;
extern UTType *const UTTypePDF;
extern UTType *const UTTypePHPScript;
extern UTType *const UTTypePKCS12;
extern UTType *const UTTypePNG;
extern UTType *const UTTypePackage;
extern UTType *const UTTypePerlScript;
extern UTType *const UTTypePlainText;
extern UTType *const UTTypePlaylist;
extern UTType *const UTTypePluginBundle;
extern UTType *const UTTypePresentation;
extern UTType *const UTTypePropertyList;
extern UTType *const UTTypePythonScript;
extern UTType *const UTTypeQuickLookGenerator;
extern UTType *const UTTypeQuickTimeMovie;
extern UTType *const UTTypeRAWImage;
extern UTType *const UTTypeRTF;
extern UTType *const UTTypeRTFD;
extern UTType *const UTTypeRealityFile;
extern UTType *const UTTypeResolvable;
extern UTType *const UTTypeRubyScript;
extern UTType *const UTTypeSVG;
extern UTType *const UTTypeSceneKitScene;
extern UTType *const UTTypeScript;
extern UTType *const UTTypeShellScript;
extern UTType *const UTTypeSourceCode;
extern UTType *const UTTypeSpotlightImporter;
extern UTType *const UTTypeSpreadsheet;
extern UTType *const UTTypeSwiftSource;
extern UTType *const UTTypeSymbolicLink;
extern UTType *const UTTypeSystemPreferencesPane;
extern UTType *const UTTypeTIFF;
extern UTType *const UTTypeTabSeparatedText;
extern UTType *const UTTypeText;
extern UTType *const UTTypeToDoItem;
extern UTType *const UTTypeURL;
extern UTType *const UTTypeURLBookmarkData;
extern UTType *const UTTypeUSD;
extern UTType *const UTTypeUSDZ;
extern UTType *const UTTypeUTF16ExternalPlainText;
extern UTType *const UTTypeUTF16PlainText;
extern UTType *const UTTypeUTF8PlainText;
extern UTType *const UTTypeUTF8TabSeparatedText;
extern UTType *const UTTypeUnixExecutable;
extern UTType *const UTTypeVCard;
extern UTType *const UTTypeVideo;
extern UTType *const UTTypeVolume;
extern UTType *const UTTypeWAV;
extern UTType *const UTTypeWebArchive;
extern UTType *const UTTypeWebP;
extern UTType *const UTTypeX509Certificate;
extern UTType *const UTTypeXML;
extern UTType *const UTTypeXMLPropertyList;
extern UTType *const UTTypeXPCService;
extern UTType *const UTTypeYAML;
extern UTType *const UTTypeZIP;
extern void* const _UTTagClassBluetoothVendorProductID;
extern void* const _UTTagClassDeviceModelCode;
extern void* const _UTTagClassHFSTypeCode;
extern void* const _UTTagClassPasteboardType;
extern void* const _UTTypeAppCategory;
extern void* const _UTTypeAppCategoryActionGames;
extern void* const _UTTypeAppCategoryAdventureGames;
extern void* const _UTTypeAppCategoryArcadeGames;
extern void* const _UTTypeAppCategoryBoardGames;
extern void* const _UTTypeAppCategoryBookmarks;
extern void* const _UTTypeAppCategoryBooks;
extern void* const _UTTypeAppCategoryBusiness;
extern void* const _UTTypeAppCategoryCardGames;
extern void* const _UTTypeAppCategoryCasinoGames;
extern void* const _UTTypeAppCategoryDeveloperTools;
extern void* const _UTTypeAppCategoryDiceGames;
extern void* const _UTTypeAppCategoryEducation;
extern void* const _UTTypeAppCategoryEducationalGames;
extern void* const _UTTypeAppCategoryEntertainment;
extern void* const _UTTypeAppCategoryFamilyGames;
extern void* const _UTTypeAppCategoryFinance;
extern void* const _UTTypeAppCategoryFoodAndDrink;
extern void* const _UTTypeAppCategoryGames;
extern void* const _UTTypeAppCategoryGraphicsDesign;
extern void* const _UTTypeAppCategoryHealthcareFitness;
extern void* const _UTTypeAppCategoryKidsGames;
extern void* const _UTTypeAppCategoryLifestyle;
extern void* const _UTTypeAppCategoryMagazinesAndNewspapers;
extern void* const _UTTypeAppCategoryMedical;
extern void* const _UTTypeAppCategoryMusic;
extern void* const _UTTypeAppCategoryMusicGames;
extern void* const _UTTypeAppCategoryNavigation;
extern void* const _UTTypeAppCategoryNews;
extern void* const _UTTypeAppCategoryPhotoAndVideo;
extern void* const _UTTypeAppCategoryPhotography;
extern void* const _UTTypeAppCategoryProductivity;
extern void* const _UTTypeAppCategoryPuzzleGames;
extern void* const _UTTypeAppCategoryRacingGames;
extern void* const _UTTypeAppCategoryReference;
extern void* const _UTTypeAppCategoryRolePlayingGames;
extern void* const _UTTypeAppCategoryShopping;
extern void* const _UTTypeAppCategorySimulationGames;
extern void* const _UTTypeAppCategorySocialNetworking;
extern void* const _UTTypeAppCategorySports;
extern void* const _UTTypeAppCategorySportsGames;
extern void* const _UTTypeAppCategoryStrategyGames;
extern void* const _UTTypeAppCategoryTravel;
extern void* const _UTTypeAppCategoryTriviaGames;
extern void* const _UTTypeAppCategoryUtilities;
extern void* const _UTTypeAppCategoryVideo;
extern void* const _UTTypeAppCategoryWeather;
extern void* const _UTTypeAppCategoryWordGames;
extern void* const _UTTypeAppleDevice;
extern void* const _UTTypeAppleEncryptedArchive;
extern void* const _UTTypeAppleTV;
extern void* const _UTTypeAppleWatch;
extern void* const _UTTypeApplicationsFolder;
extern void* const _UTTypeBlockSpecial;
extern void* const _UTTypeCharacterSpecial;
extern void* const _UTTypeComputer;
extern void* const _UTTypeDataContainer;
extern void* const _UTTypeDevice;
extern void* const _UTTypeDisplay;
extern void* const _UTTypeDropFolder;
extern void* const _UTTypeGenericPC;
extern void* const _UTTypeHEIFStandard;
extern void* const _UTTypeHomePod;
extern void* const _UTTypeLibraryFolder;
extern void* const _UTTypeMac;
extern void* const _UTTypeMacBook;
extern void* const _UTTypeMacBookAir;
extern void* const _UTTypeMacBookPro;
extern void* const _UTTypeMacLaptop;
extern void* const _UTTypeMacMini;
extern void* const _UTTypeMacPro;
extern void* const _UTTypeNamedPipeOrFIFO;
extern void* const _UTTypeNetworkNeighborhood;
extern void* const _UTTypePassBundle;
extern void* const _UTTypePassData;
extern void* const _UTTypePassesData;
extern void* const _UTTypeServersFolder;
extern void* const _UTTypeSocket;
extern void* const _UTTypeSpeaker;
extern void* const _UTTypeiMac;
extern void* const _UTTypeiOSDevice;
extern void* const _UTTypeiOSSimulator;
extern void* const _UTTypeiPad;
extern void* const _UTTypeiPhone;
extern void* const _UTTypeiPodTouch;
extern void* const _ZTSSt11logic_error;
extern void* const _ZTSSt12length_error;
extern void* const _ZTSSt19bad_optional_access;
extern void* const _ZTSSt20bad_array_new_length;
extern void* const _ZTSSt9bad_alloc;
extern void* const _ZTSSt9exception;

#endif

