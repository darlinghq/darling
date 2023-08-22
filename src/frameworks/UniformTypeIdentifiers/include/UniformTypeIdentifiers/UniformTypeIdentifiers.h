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


extern void* const UTTagClassFilenameExtension;
extern void* const UTTagClassMIMEType;
extern void* const UTType3DContent;
extern void* const UTTypeAIFF;
extern void* const UTTypeARReferenceObject;
extern void* const UTTypeAVI;
extern void* const UTTypeAliasFile;
extern void* const UTTypeAppleArchive;
extern void* const UTTypeAppleProtectedMPEG4Audio;
extern void* const UTTypeAppleProtectedMPEG4Video;
extern void* const UTTypeAppleScript;
extern void* const UTTypeApplication;
extern void* const UTTypeApplicationBundle;
extern void* const UTTypeApplicationExtension;
extern void* const UTTypeArchive;
extern void* const UTTypeAssemblyLanguageSource;
extern void* const UTTypeAudio;
extern void* const UTTypeAudiovisualContent;
extern void* const UTTypeBMP;
extern void* const UTTypeBZ2;
extern void* const UTTypeBinaryPropertyList;
extern void* const UTTypeBookmark;
extern void* const UTTypeBundle;
extern void* const UTTypeCHeader;
extern void* const UTTypeCPlusPlusHeader;
extern void* const UTTypeCPlusPlusSource;
extern void* const UTTypeCSource;
extern void* const UTTypeCalendarEvent;
extern void* const UTTypeCommaSeparatedText;
extern void* const UTTypeCompositeContent;
extern void* const UTTypeContact;
extern void* const UTTypeContent;
extern void* const UTTypeData;
extern void* const UTTypeDatabase;
extern void* const UTTypeDelimitedText;
extern void* const UTTypeDirectory;
extern void* const UTTypeDiskImage;
extern void* const UTTypeEPUB;
extern void* const UTTypeEXE;
extern void* const UTTypeEmailMessage;
extern void* const UTTypeExecutable;
extern void* const UTTypeFileURL;
extern void* const UTTypeFlatRTFD;
extern void* const UTTypeFolder;
extern void* const UTTypeFont;
extern void* const UTTypeFramework;
extern void* const UTTypeGIF;
extern void* const UTTypeGZIP;
extern void* const UTTypeHEIC;
extern void* const UTTypeHEIF;
extern void* const UTTypeHTML;
extern void* const UTTypeICNS;
extern void* const UTTypeICO;
extern void* const UTTypeImage;
extern void* const UTTypeInternetLocation;
extern void* const UTTypeInternetShortcut;
extern void* const UTTypeItem;
extern void* const UTTypeJPEG;
extern void* const UTTypeJSON;
extern void* const UTTypeJavaScript;
extern void* const UTTypeLivePhoto;
extern void* const UTTypeLog;
extern void* const UTTypeM3UPlaylist;
extern void* const UTTypeMIDI;
extern void* const UTTypeMP3;
extern void* const UTTypeMPEG;
extern void* const UTTypeMPEG2TransportStream;
extern void* const UTTypeMPEG2Video;
extern void* const UTTypeMPEG4Audio;
extern void* const UTTypeMPEG4Movie;
extern void* const UTTypeMakefile;
extern void* const UTTypeMessage;
extern void* const UTTypeMountPoint;
extern void* const UTTypeMovie;
extern void* const UTTypeOSAScript;
extern void* const UTTypeOSAScriptBundle;
extern void* const UTTypeObjectiveCPlusPlusSource;
extern void* const UTTypeObjectiveCSource;
extern void* const UTTypePDF;
extern void* const UTTypePHPScript;
extern void* const UTTypePKCS12;
extern void* const UTTypePNG;
extern void* const UTTypePackage;
extern void* const UTTypePerlScript;
extern void* const UTTypePlainText;
extern void* const UTTypePlaylist;
extern void* const UTTypePluginBundle;
extern void* const UTTypePresentation;
extern void* const UTTypePropertyList;
extern void* const UTTypePythonScript;
extern void* const UTTypeQuickLookGenerator;
extern void* const UTTypeQuickTimeMovie;
extern void* const UTTypeRAWImage;
extern void* const UTTypeRTF;
extern void* const UTTypeRTFD;
extern void* const UTTypeRealityFile;
extern void* const UTTypeResolvable;
extern void* const UTTypeRubyScript;
extern void* const UTTypeSVG;
extern void* const UTTypeSceneKitScene;
extern void* const UTTypeScript;
extern void* const UTTypeShellScript;
extern void* const UTTypeSourceCode;
extern void* const UTTypeSpotlightImporter;
extern void* const UTTypeSpreadsheet;
extern void* const UTTypeSwiftSource;
extern void* const UTTypeSymbolicLink;
extern void* const UTTypeSystemPreferencesPane;
extern void* const UTTypeTIFF;
extern void* const UTTypeTabSeparatedText;
extern void* const UTTypeText;
extern void* const UTTypeToDoItem;
extern void* const UTTypeURL;
extern void* const UTTypeURLBookmarkData;
extern void* const UTTypeUSD;
extern void* const UTTypeUSDZ;
extern void* const UTTypeUTF16ExternalPlainText;
extern void* const UTTypeUTF16PlainText;
extern void* const UTTypeUTF8PlainText;
extern void* const UTTypeUTF8TabSeparatedText;
extern void* const UTTypeUnixExecutable;
extern void* const UTTypeVCard;
extern void* const UTTypeVideo;
extern void* const UTTypeVolume;
extern void* const UTTypeWAV;
extern void* const UTTypeWebArchive;
extern void* const UTTypeWebP;
extern void* const UTTypeX509Certificate;
extern void* const UTTypeXML;
extern void* const UTTypeXMLPropertyList;
extern void* const UTTypeXPCService;
extern void* const UTTypeYAML;
extern void* const UTTypeZIP;
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

