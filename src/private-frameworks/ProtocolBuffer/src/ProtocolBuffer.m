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


#include <ProtocolBuffer/ProtocolBuffer.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void PBDataWriterPlaceMark(PBDataWriter* writer, PBDataReaderMark* mark, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterPlaceMark called");
    return;
}

void PBDataWriterRecallMark(PBDataWriter* writer, PBDataReaderMark* mark, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterRecallMark called");
    return;
}

void PBDataWriterWriteBOOLField(PBDataWriter* writer, BOOL boolean, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteBOOLField called");
    return;
}

void PBDataWriterWriteBareVarint(PBDataWriter* writer, void* value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteBareVarint called");
    return;
}

void PBDataWriterWriteDataField(PBDataWriter* writer, NSData* value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteDataField called");
    return;
}

void PBDataWriterWriteDoubleField(PBDataWriter* writer, double value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteDoubleField called");
    return;
}

void PBDataWriterWriteFixed32Field(PBDataWriter* writer, int32_t value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteFixed32Field called");
    return;
}

void PBDataWriterWriteFixed64Field(PBDataWriter* writer, int64_t value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteFixed64Field called");
    return;
}

void PBDataWriterWriteFloatField(PBDataWriter* writer, float value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteFloatField called");
    return;
}

void PBDataWriterWriteInt32Field(PBDataWriter* writer, int32_t value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteInt32Field called");
    return;
}

void PBDataWriterWriteInt64Field(PBDataWriter* writer, int64_t value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteInt64Field called");
    return;
}

void PBDataWriterWriteSfixed32Field(PBDataWriter* writer, int32_t value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteSfixed32Field called");
    return;
}

void PBDataWriterWriteSfixed64Field(PBDataWriter* writer, int64_t value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteSfixed64Field called");
    return;
}

void PBDataWriterWriteSint32Field(PBDataWriter* writer, int32_t value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteSint32Field called");
    return;
}

void PBDataWriterWriteSint64Field(PBDataWriter* writer, int64_t value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteSint64Field called");
    return;
}

void PBDataWriterWriteStringField(PBDataWriter* writer, NSString* string, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteStringField called");
    return;
}

void PBDataWriterWriteSubgroup(PBDataWriter* writer, void* group, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteSubgroup called");
    return;
}

void PBDataWriterWriteSubmessage(PBDataWriter* writer, PBCodable* codable, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteSubmessage called");
    return;
}

void PBDataWriterWriteUint32Field(PBDataWriter* writer, uint32_t value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteUint32Field called");
    return;
}

void PBDataWriterWriteUint64Field(PBDataWriter* writer, uint64_t value, uint32_t tag)
{
    if (verbose) puts("STUB: PBDataWriterWriteUint64Field called");
    return;
}

void* PBHashBytes(void)
{
    if (verbose) puts("STUB: PBHashBytes called");
    return NULL;
}

BOOL PBReaderPlaceMark(PBDataReader* reader, PBDataReaderMark* mark)
{
    if (verbose) puts("STUB: PBReaderPlaceMark called");
    return FALSE;
}

uint16_t PBReaderReadBigEndianFixed16(PBDataReader* reader)
{
    if (verbose) puts("STUB: PBReaderReadBigEndianFixed16 called");
    return 0;
}

uint32_t PBReaderReadBigEndianFixed32(PBDataReader* reader)
{
    if (verbose) puts("STUB: PBReaderReadBigEndianFixed32 called");
    return 0;
}

uint64_t PBReaderReadBigEndianFixed64(PBDataReader* reader)
{
    if (verbose) puts("STUB: PBReaderReadBigEndianFixed64 called");
    return 0;
}

NSData* PBReaderReadData(PBDataReader* reader)
{
    if (verbose) puts("STUB: PBReaderReadData called");
    return NULL;
}

NSString* PBReaderReadString(PBDataReader* reader)
{
    if (verbose) puts("STUB: PBReaderReadString called");
    return NULL;
}

void* PBReaderReadVarIntBuf(PBDataReader* reader)
{
    if (verbose) puts("STUB: PBReaderReadVarIntBuf called");
    return NULL;
}

void PBReaderRecallMark(PBDataReader* reader, PBDataReaderMark* mark)
{
    if (verbose) puts("STUB: PBReaderRecallMark called");
    return;
}

BOOL PBReaderSkipValueWithTag(PBDataReader* reader, uint32_t tag, uint8_t type)
{
    if (verbose) puts("STUB: PBReaderSkipValueWithTag called");
    return FALSE;
}

void* PBRepeatedBOOLAdd(void)
{
    if (verbose) puts("STUB: PBRepeatedBOOLAdd called");
    return NULL;
}

void* PBRepeatedBOOLClear(void)
{
    if (verbose) puts("STUB: PBRepeatedBOOLClear called");
    return NULL;
}

void* PBRepeatedBOOLCopy(void)
{
    if (verbose) puts("STUB: PBRepeatedBOOLCopy called");
    return NULL;
}

void* PBRepeatedBOOLHash(void)
{
    if (verbose) puts("STUB: PBRepeatedBOOLHash called");
    return NULL;
}

void* PBRepeatedBOOLIsEqual(void)
{
    if (verbose) puts("STUB: PBRepeatedBOOLIsEqual called");
    return NULL;
}

void* PBRepeatedBOOLNSArray(void)
{
    if (verbose) puts("STUB: PBRepeatedBOOLNSArray called");
    return NULL;
}

void* PBRepeatedBOOLSet(void)
{
    if (verbose) puts("STUB: PBRepeatedBOOLSet called");
    return NULL;
}

void* PBRepeatedDoubleAdd(void)
{
    if (verbose) puts("STUB: PBRepeatedDoubleAdd called");
    return NULL;
}

void* PBRepeatedDoubleClear(void)
{
    if (verbose) puts("STUB: PBRepeatedDoubleClear called");
    return NULL;
}

void* PBRepeatedDoubleCopy(void)
{
    if (verbose) puts("STUB: PBRepeatedDoubleCopy called");
    return NULL;
}

void* PBRepeatedDoubleHash(void)
{
    if (verbose) puts("STUB: PBRepeatedDoubleHash called");
    return NULL;
}

void* PBRepeatedDoubleIsEqual(void)
{
    if (verbose) puts("STUB: PBRepeatedDoubleIsEqual called");
    return NULL;
}

void* PBRepeatedDoubleNSArray(void)
{
    if (verbose) puts("STUB: PBRepeatedDoubleNSArray called");
    return NULL;
}

void* PBRepeatedDoubleSet(void)
{
    if (verbose) puts("STUB: PBRepeatedDoubleSet called");
    return NULL;
}

void* PBRepeatedFloatAdd(void)
{
    if (verbose) puts("STUB: PBRepeatedFloatAdd called");
    return NULL;
}

void* PBRepeatedFloatClear(void)
{
    if (verbose) puts("STUB: PBRepeatedFloatClear called");
    return NULL;
}

void* PBRepeatedFloatCopy(void)
{
    if (verbose) puts("STUB: PBRepeatedFloatCopy called");
    return NULL;
}

void* PBRepeatedFloatHash(void)
{
    if (verbose) puts("STUB: PBRepeatedFloatHash called");
    return NULL;
}

void* PBRepeatedFloatIsEqual(void)
{
    if (verbose) puts("STUB: PBRepeatedFloatIsEqual called");
    return NULL;
}

void* PBRepeatedFloatNSArray(void)
{
    if (verbose) puts("STUB: PBRepeatedFloatNSArray called");
    return NULL;
}

void* PBRepeatedFloatSet(void)
{
    if (verbose) puts("STUB: PBRepeatedFloatSet called");
    return NULL;
}

void* PBRepeatedInt32Add(void)
{
    if (verbose) puts("STUB: PBRepeatedInt32Add called");
    return NULL;
}

void* PBRepeatedInt32Clear(void)
{
    if (verbose) puts("STUB: PBRepeatedInt32Clear called");
    return NULL;
}

void* PBRepeatedInt32Copy(void)
{
    if (verbose) puts("STUB: PBRepeatedInt32Copy called");
    return NULL;
}

void* PBRepeatedInt32Hash(void)
{
    if (verbose) puts("STUB: PBRepeatedInt32Hash called");
    return NULL;
}

void* PBRepeatedInt32IsEqual(void)
{
    if (verbose) puts("STUB: PBRepeatedInt32IsEqual called");
    return NULL;
}

void* PBRepeatedInt32NSArray(void)
{
    if (verbose) puts("STUB: PBRepeatedInt32NSArray called");
    return NULL;
}

void* PBRepeatedInt32Set(void)
{
    if (verbose) puts("STUB: PBRepeatedInt32Set called");
    return NULL;
}

void* PBRepeatedInt64Add(void)
{
    if (verbose) puts("STUB: PBRepeatedInt64Add called");
    return NULL;
}

void* PBRepeatedInt64Clear(void)
{
    if (verbose) puts("STUB: PBRepeatedInt64Clear called");
    return NULL;
}

void* PBRepeatedInt64Copy(void)
{
    if (verbose) puts("STUB: PBRepeatedInt64Copy called");
    return NULL;
}

void* PBRepeatedInt64Hash(void)
{
    if (verbose) puts("STUB: PBRepeatedInt64Hash called");
    return NULL;
}

void* PBRepeatedInt64IsEqual(void)
{
    if (verbose) puts("STUB: PBRepeatedInt64IsEqual called");
    return NULL;
}

void* PBRepeatedInt64NSArray(void)
{
    if (verbose) puts("STUB: PBRepeatedInt64NSArray called");
    return NULL;
}

void* PBRepeatedInt64Set(void)
{
    if (verbose) puts("STUB: PBRepeatedInt64Set called");
    return NULL;
}

void* PBRepeatedUInt32Add(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt32Add called");
    return NULL;
}

void* PBRepeatedUInt32Clear(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt32Clear called");
    return NULL;
}

void* PBRepeatedUInt32Copy(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt32Copy called");
    return NULL;
}

void* PBRepeatedUInt32Hash(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt32Hash called");
    return NULL;
}

void* PBRepeatedUInt32IsEqual(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt32IsEqual called");
    return NULL;
}

void* PBRepeatedUInt32NSArray(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt32NSArray called");
    return NULL;
}

void* PBRepeatedUInt32Set(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt32Set called");
    return NULL;
}

void* PBRepeatedUInt64Add(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt64Add called");
    return NULL;
}

void* PBRepeatedUInt64Clear(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt64Clear called");
    return NULL;
}

void* PBRepeatedUInt64Copy(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt64Copy called");
    return NULL;
}

void* PBRepeatedUInt64Hash(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt64Hash called");
    return NULL;
}

void* PBRepeatedUInt64IsEqual(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt64IsEqual called");
    return NULL;
}

void* PBRepeatedUInt64NSArray(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt64NSArray called");
    return NULL;
}

void* PBRepeatedUInt64Set(void)
{
    if (verbose) puts("STUB: PBRepeatedUInt64Set called");
    return NULL;
}

void* PBUnknownFieldAdd(void)
{
    if (verbose) puts("STUB: PBUnknownFieldAdd called");
    return NULL;
}

void* PBUnknownFieldsSetMaximumRecursionDepth(void)
{
    if (verbose) puts("STUB: PBUnknownFieldsSetMaximumRecursionDepth called");
    return NULL;
}

void* _ZN2PB13TextFormatter11beginObjectEPKc(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter11beginObjectEPKc called");
    return NULL;
}

void* _ZN2PB13TextFormatter6dedentEv(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter6dedentEv called");
    return NULL;
}

void* _ZN2PB13TextFormatter6formatEPKcRKNS_4DataE(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter6formatEPKcRKNS_4DataE called");
    return NULL;
}

void* _ZN2PB13TextFormatter6formatEPKcRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter6formatEPKcRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN2PB13TextFormatter6formatEPKcRKNSt3__16vectorIhNS3_9allocatorIhEEEE(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter6formatEPKcRKNSt3__16vectorIhNS3_9allocatorIhEEEE called");
    return NULL;
}

void* _ZN2PB13TextFormatter6formatEPKcb(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter6formatEPKcb called");
    return NULL;
}

void* _ZN2PB13TextFormatter6formatEPKcd(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter6formatEPKcd called");
    return NULL;
}

void* _ZN2PB13TextFormatter6formatEPKcf(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter6formatEPKcf called");
    return NULL;
}

void* _ZN2PB13TextFormatter6formatEPKci(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter6formatEPKci called");
    return NULL;
}

void* _ZN2PB13TextFormatter6formatEPKcj(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter6formatEPKcj called");
    return NULL;
}

void* _ZN2PB13TextFormatter6formatEPKcx(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter6formatEPKcx called");
    return NULL;
}

void* _ZN2PB13TextFormatter6formatEPKcy(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter6formatEPKcy called");
    return NULL;
}

void* _ZN2PB13TextFormatter6indentEv(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter6indentEv called");
    return NULL;
}

void* _ZN2PB13TextFormatter9endObjectEv(void)
{
    if (verbose) puts("STUB: _ZN2PB13TextFormatter9endObjectEv called");
    return NULL;
}

void* _ZN2PB4BaseD0Ev(void)
{
    if (verbose) puts("STUB: _ZN2PB4BaseD0Ev called");
    return NULL;
}

void* _ZN2PB4BaseD1Ev(void)
{
    if (verbose) puts("STUB: _ZN2PB4BaseD1Ev called");
    return NULL;
}

void* _ZN2PB4BaseD2Ev(void)
{
    if (verbose) puts("STUB: _ZN2PB4BaseD2Ev called");
    return NULL;
}

void* _ZN2PB4Data5clearEv(void)
{
    if (verbose) puts("STUB: _ZN2PB4Data5clearEv called");
    return NULL;
}

void* _ZN2PB4Data6assignEPKhS2_(void)
{
    if (verbose) puts("STUB: _ZN2PB4Data6assignEPKhS2_ called");
    return NULL;
}

void* _ZN2PB4DataC1EPKhm(void)
{
    if (verbose) puts("STUB: _ZN2PB4DataC1EPKhm called");
    return NULL;
}

void* _ZN2PB4DataC1ERKS0_(void)
{
    if (verbose) puts("STUB: _ZN2PB4DataC1ERKS0_ called");
    return NULL;
}

void* _ZN2PB4DataC2EPKhm(void)
{
    if (verbose) puts("STUB: _ZN2PB4DataC2EPKhm called");
    return NULL;
}

void* _ZN2PB4DataC2ERKS0_(void)
{
    if (verbose) puts("STUB: _ZN2PB4DataC2ERKS0_ called");
    return NULL;
}

void* _ZN2PB6Reader10recallMarkERKNS_10ReaderMarkE(void)
{
    if (verbose) puts("STUB: _ZN2PB6Reader10recallMarkERKNS_10ReaderMarkE called");
    return NULL;
}

void* _ZN2PB6Reader4readERNS_4DataE(void)
{
    if (verbose) puts("STUB: _ZN2PB6Reader4readERNS_4DataE called");
    return NULL;
}

void* _ZN2PB6Reader4readERNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN2PB6Reader4readERNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN2PB6Reader4readERNSt3__16vectorIhNS1_9allocatorIhEEEE(void)
{
    if (verbose) puts("STUB: _ZN2PB6Reader4readERNSt3__16vectorIhNS1_9allocatorIhEEEE called");
    return NULL;
}

void* _ZN2PB6Reader4skipEjhi(void)
{
    if (verbose) puts("STUB: _ZN2PB6Reader4skipEjhi called");
    return NULL;
}

void* _ZN2PB6Reader9placeMarkERNS_10ReaderMarkE(void)
{
    if (verbose) puts("STUB: _ZN2PB6Reader9placeMarkERNS_10ReaderMarkE called");
    return NULL;
}

void* _ZN2PB6ReaderC1EPKhm(void)
{
    if (verbose) puts("STUB: _ZN2PB6ReaderC1EPKhm called");
    return NULL;
}

void* _ZN2PB6ReaderC2EPKhm(void)
{
    if (verbose) puts("STUB: _ZN2PB6ReaderC2EPKhm called");
    return NULL;
}

void* _ZN2PB6Writer10recallMarkERKNS_10WriterMarkE(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer10recallMarkERKNS_10WriterMarkE called");
    return NULL;
}

void* _ZN2PB6Writer10writeFixedEij(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer10writeFixedEij called");
    return NULL;
}

void* _ZN2PB6Writer10writeFixedEjj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer10writeFixedEjj called");
    return NULL;
}

void* _ZN2PB6Writer10writeFixedExj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer10writeFixedExj called");
    return NULL;
}

void* _ZN2PB6Writer10writeFixedEyj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer10writeFixedEyj called");
    return NULL;
}

void* _ZN2PB6Writer11writeVarIntEij(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer11writeVarIntEij called");
    return NULL;
}

void* _ZN2PB6Writer11writeVarIntEjj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer11writeVarIntEjj called");
    return NULL;
}

void* _ZN2PB6Writer11writeVarIntExj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer11writeVarIntExj called");
    return NULL;
}

void* _ZN2PB6Writer11writeVarIntEyj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer11writeVarIntEyj called");
    return NULL;
}

void* _ZN2PB6Writer13writeSubgroupERKNS_4BaseEj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer13writeSubgroupERKNS_4BaseEj called");
    return NULL;
}

void* _ZN2PB6Writer15writeSubmessageERKNS_4BaseEj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer15writeSubmessageERKNS_4BaseEj called");
    return NULL;
}

void* _ZN2PB6Writer5writeERKNS_4DataEj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer5writeERKNS_4DataEj called");
    return NULL;
}

void* _ZN2PB6Writer5writeERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer5writeERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEj called");
    return NULL;
}

void* _ZN2PB6Writer5writeERKNSt3__16vectorIhNS1_9allocatorIhEEEEj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer5writeERKNSt3__16vectorIhNS1_9allocatorIhEEEEj called");
    return NULL;
}

void* _ZN2PB6Writer5writeEbj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer5writeEbj called");
    return NULL;
}

void* _ZN2PB6Writer5writeEdj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer5writeEdj called");
    return NULL;
}

void* _ZN2PB6Writer5writeEfj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer5writeEfj called");
    return NULL;
}

void* _ZN2PB6Writer9placeMarkERNS_10WriterMarkEj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer9placeMarkERNS_10WriterMarkEj called");
    return NULL;
}

void* _ZN2PB6Writer9writeSIntEij(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer9writeSIntEij called");
    return NULL;
}

void* _ZN2PB6Writer9writeSIntExj(void)
{
    if (verbose) puts("STUB: _ZN2PB6Writer9writeSIntExj called");
    return NULL;
}

void* _ZN2PB6WriterC1Ev(void)
{
    if (verbose) puts("STUB: _ZN2PB6WriterC1Ev called");
    return NULL;
}

void* _ZN2PB6WriterC2Ev(void)
{
    if (verbose) puts("STUB: _ZN2PB6WriterC2Ev called");
    return NULL;
}

void* _ZN2PB6WriterD1Ev(void)
{
    if (verbose) puts("STUB: _ZN2PB6WriterD1Ev called");
    return NULL;
}

void* _ZN2PB6WriterD2Ev(void)
{
    if (verbose) puts("STUB: _ZN2PB6WriterD2Ev called");
    return NULL;
}

void* _ZNK2PB4Base13formattedTextEv(void)
{
    if (verbose) puts("STUB: _ZNK2PB4Base13formattedTextEv called");
    return NULL;
}

void* _ZNK2PB4DataeqERKS0_(void)
{
    if (verbose) puts("STUB: _ZNK2PB4DataeqERKS0_ called");
    return NULL;
}
