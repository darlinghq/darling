/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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


#ifndef _ProtocolBuffer_H_
#define _ProtocolBuffer_H_

#import <Foundation/Foundation.h>

#import <ProtocolBuffer/NSURLSessionDataDelegate.h>
#import <ProtocolBuffer/NSURLSessionDelegate.h>
#import <ProtocolBuffer/PBDataReader.h>
#import <ProtocolBuffer/PBTextWriter.h>
#import <ProtocolBuffer/PBMutableData.h>
#import <ProtocolBuffer/PBDataWriter.h>
#import <ProtocolBuffer/PBTextReader.h>
#import <ProtocolBuffer/PBCodable.h>
#import <ProtocolBuffer/PBRequest.h>
#import <ProtocolBuffer/PBStreamWriter.h>
#import <ProtocolBuffer/PBMessageStreamReader.h>
#import <ProtocolBuffer/PBMessageStreamWriter.h>
#import <ProtocolBuffer/PBStreamReader.h>
#import <ProtocolBuffer/_PBProperty.h>
#import <ProtocolBuffer/PBUnknownFields.h>
#import <ProtocolBuffer/PBSessionRequester.h>

void* PBRepeatedBOOLAdd(void);
void* PBRepeatedBOOLClear(void);
void* PBRepeatedBOOLCopy(void);
void* PBRepeatedBOOLHash(void);
void* PBRepeatedBOOLIsEqual(void);
void* PBRepeatedBOOLNSArray(void);
void* PBRepeatedBOOLSet(void);
void* PBRepeatedDoubleAdd(void);
void* PBRepeatedDoubleClear(void);
void* PBRepeatedDoubleCopy(void);
void* PBRepeatedDoubleHash(void);
void* PBRepeatedDoubleIsEqual(void);
void* PBRepeatedDoubleNSArray(void);
void* PBRepeatedDoubleSet(void);
void* PBRepeatedFloatAdd(void);
void* PBRepeatedFloatClear(void);
void* PBRepeatedFloatCopy(void);
void* PBRepeatedFloatHash(void);
void* PBRepeatedFloatIsEqual(void);
void* PBRepeatedFloatNSArray(void);
void* PBRepeatedFloatSet(void);
void* PBRepeatedInt32Add(void);
void* PBRepeatedInt32Clear(void);
void* PBRepeatedInt32Copy(void);
void* PBRepeatedInt32Hash(void);
void* PBRepeatedInt32IsEqual(void);
void* PBRepeatedInt32NSArray(void);
void* PBRepeatedInt32Set(void);
void* PBRepeatedInt64Add(void);
void* PBRepeatedInt64Clear(void);
void* PBRepeatedInt64Copy(void);
void* PBRepeatedInt64Hash(void);
void* PBRepeatedInt64IsEqual(void);
void* PBRepeatedInt64NSArray(void);
void* PBRepeatedInt64Set(void);
void* PBRepeatedUInt32Add(void);
void* PBRepeatedUInt32Clear(void);
void* PBRepeatedUInt32Copy(void);
void* PBRepeatedUInt32Hash(void);
void* PBRepeatedUInt32IsEqual(void);
void* PBRepeatedUInt32NSArray(void);
void* PBRepeatedUInt32Set(void);
void* PBRepeatedUInt64Add(void);
void* PBRepeatedUInt64Clear(void);
void* PBRepeatedUInt64Copy(void);
void* PBRepeatedUInt64Hash(void);
void* PBRepeatedUInt64IsEqual(void);
void* PBRepeatedUInt64NSArray(void);
void* PBRepeatedUInt64Set(void);
void* PBUnknownFieldAdd(void);
void* PBUnknownFieldsSetMaximumRecursionDepth(void);
void* _ZN2PB13TextFormatter11beginObjectEPKc(void);
void* _ZN2PB13TextFormatter6dedentEv(void);
void* _ZN2PB13TextFormatter6formatEPKcRKNS_4DataE(void);
void* _ZN2PB13TextFormatter6formatEPKcRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE(void);
void* _ZN2PB13TextFormatter6formatEPKcRKNSt3__16vectorIhNS3_9allocatorIhEEEE(void);
void* _ZN2PB13TextFormatter6formatEPKcb(void);
void* _ZN2PB13TextFormatter6formatEPKcd(void);
void* _ZN2PB13TextFormatter6formatEPKcf(void);
void* _ZN2PB13TextFormatter6formatEPKci(void);
void* _ZN2PB13TextFormatter6formatEPKcj(void);
void* _ZN2PB13TextFormatter6formatEPKcx(void);
void* _ZN2PB13TextFormatter6formatEPKcy(void);
void* _ZN2PB13TextFormatter6indentEv(void);
void* _ZN2PB13TextFormatter9endObjectEv(void);
void* _ZN2PB4BaseD0Ev(void);
void* _ZN2PB4BaseD1Ev(void);
void* _ZN2PB4BaseD2Ev(void);
void* _ZN2PB4Data5clearEv(void);
void* _ZN2PB4Data6assignEPKhS2_(void);
void* _ZN2PB4DataC1EPKhm(void);
void* _ZN2PB4DataC1ERKS0_(void);
void* _ZN2PB4DataC2EPKhm(void);
void* _ZN2PB4DataC2ERKS0_(void);
void* _ZN2PB6Reader10recallMarkERKNS_10ReaderMarkE(void);
void* _ZN2PB6Reader4readERNS_4DataE(void);
void* _ZN2PB6Reader4readERNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void);
void* _ZN2PB6Reader4readERNSt3__16vectorIhNS1_9allocatorIhEEEE(void);
void* _ZN2PB6Reader4skipEjhi(void);
void* _ZN2PB6Reader9placeMarkERNS_10ReaderMarkE(void);
void* _ZN2PB6ReaderC1EPKhm(void);
void* _ZN2PB6ReaderC2EPKhm(void);
void* _ZN2PB6Writer10recallMarkERKNS_10WriterMarkE(void);
void* _ZN2PB6Writer10writeFixedEij(void);
void* _ZN2PB6Writer10writeFixedEjj(void);
void* _ZN2PB6Writer10writeFixedExj(void);
void* _ZN2PB6Writer10writeFixedEyj(void);
void* _ZN2PB6Writer11writeVarIntEij(void);
void* _ZN2PB6Writer11writeVarIntEjj(void);
void* _ZN2PB6Writer11writeVarIntExj(void);
void* _ZN2PB6Writer11writeVarIntEyj(void);
void* _ZN2PB6Writer13writeSubgroupERKNS_4BaseEj(void);
void* _ZN2PB6Writer15writeSubmessageERKNS_4BaseEj(void);
void* _ZN2PB6Writer5writeERKNS_4DataEj(void);
void* _ZN2PB6Writer5writeERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEj(void);
void* _ZN2PB6Writer5writeERKNSt3__16vectorIhNS1_9allocatorIhEEEEj(void);
void* _ZN2PB6Writer5writeEbj(void);
void* _ZN2PB6Writer5writeEdj(void);
void* _ZN2PB6Writer5writeEfj(void);
void* _ZN2PB6Writer9placeMarkERNS_10WriterMarkEj(void);
void* _ZN2PB6Writer9writeSIntEij(void);
void* _ZN2PB6Writer9writeSIntExj(void);
void* _ZN2PB6WriterC1Ev(void);
void* _ZN2PB6WriterC2Ev(void);
void* _ZN2PB6WriterD1Ev(void);
void* _ZN2PB6WriterD2Ev(void);
void* _ZNK2PB4Base13formattedTextEv(void);
void* _ZNK2PB4DataeqERKS0_(void);

#endif
