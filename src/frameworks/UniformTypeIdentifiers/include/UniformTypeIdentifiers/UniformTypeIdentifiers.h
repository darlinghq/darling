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


void *_ZN22UniformTypeIdentifiers9ModelCodeL26getDeviceTypeWithModelCodeEP8NSStringRKNSt3__18optionalI15UTHardwareColorEENS0_7OptionsE(void);
void *_ZN22UniformTypeIdentifiers9ModelCodeL71getDeviceTypeWithModelCodeAndHardwareColorWithoutResolvingCurrentDeviceEP8NSStringRKNSt3__18optionalI15UTHardwareColorEE(void);
void *_ZN22UniformTypeIdentifiers9ModelCodeL26getCurrentDeviceModelCodesEv(void);
void *_ZN22UniformTypeIdentifiers9ModelCodeL32getHardwareColorForCurrentDeviceEv(void);
void *_ZN22UniformTypeIdentifiers9ModelCodeL22getHardwareColorSuffixERK15UTHardwareColor(void);
void *_ZNSt3__127__throw_bad_optional_accessEv(void);
void *_ZN22UniformTypeIdentifiers9ModelCodeL3logEv(void);
void *_ZN22UniformTypeIdentifiers9AccessoryL3logEv(void);
void *_ZL23_UTTypeGetForIdentifierP8NSStringa(void);
void *_ZN22UniformTypeIdentifiers4TypeL24detachTypeRecordsInTypesIU8__strongP12NSMutableSetIP6UTTypeEEEvT_(void);
void *_UTDetachTypeRecords(void);
void *_ZN22UniformTypeIdentifiers4TypeL10isTagValidEP8NSStringS2_(void);
void *_ZN22UniformTypeIdentifiers4TypeL15getExportedTypeEP8NSStringP6UTTypeP8NSBundle(void);
void *_ZN22UniformTypeIdentifiers4TypeL15getImportedTypeEP8NSStringP6UTTypeP8NSBundle(void);
void *_ZN22UniformTypeIdentifiers4TypeL21getHFSTypeCodeFromTagEP8NSStringPj(void);
void *_ZNSt3__16vectorIP12UTTypeRecordNS_9allocatorIS3_EEEC2EmRKS2_(void);
void *__clang_call_terminate(void);
void *_ZNKSt3__113__vector_baseIP12UTTypeRecordNS_9allocatorIS3_EEE20__throw_length_errorEv(void);
void *_ZNSt3__16vectorIcNS_9allocatorIcEEEC2EmRKc(void);
void *_ZNKSt3__113__vector_baseIcNS_9allocatorIcEEE20__throw_length_errorEv(void);
void *_ZNSt3__121__murmur2_or_cityhashImLm64EEclEPKvm(void);
void *_UTCoreTypesConstantSetTypeRecordIfNil(void);
void *_ZN22UniformTypeIdentifiers9CoreTypesL3logEv(void);
void *__UNIFORM_TYPE_IDENTIFIERS_FAILED_TO_REALIZE_A_COMPILE_TIME_CONSTANT_TYPE_OBJECT__(void);
void *_ZNKSt3__113__vector_baseIP9objc_ivarNS_9allocatorIS2_EEE20__throw_length_errorEv(void);
void *_ZNSt3__120__throw_length_errorEPKc(void);
void *_ZNSt12length_errorC1EPKc(void);
void *_typeIdentifierFromUTType(void);
void *_utTypesFromTypeIdentifiers(void);
void *_ZN22UniformTypeIdentifiers13RuntimeIssuesL27checkListsForTypeIdentifierEP8NSStringP7NSArrayIP12NSDictionaryIS2_P11objc_objectEESA_(void);
void *_ZN22UniformTypeIdentifiers13RuntimeIssuesL15runtimeIssueLogEv(void);
void *_ZN22UniformTypeIdentifiers13RuntimeIssuesL43enumerateTypeIdentifiersInDeclarationsArrayEP7NSArrayIP12NSDictionaryIP8NSStringP11objc_objectEEU13block_pointerFvS4_PaE(void);
void *_ZN22UniformTypeIdentifiers13RuntimeIssuesL17isBinaryTheCallerEPK11mach_header(void);


extern extern const void* UniformTypeIdentifiersVersionString;
extern extern const void* UniformTypeIdentifiersVersionNumber;

#endif

