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

#include <Foundation/Foundation.h>

typedef int PBDataReaderMark;

@class PBCodable;

@interface PBDataWriter : NSObject

@end

void PBDataWriterWriteStringField(PBDataWriter* writer, NSString* string, uint32_t tag);
void PBDataWriterWriteBOOLField(PBDataWriter* writer, BOOL boolean, uint32_t tag);

void PBDataWriterPlaceMark(PBDataWriter* writer, PBDataReaderMark* mark, uint32_t tag);
void PBDataWriterRecallMark(PBDataWriter* writer, PBDataReaderMark* mark, uint32_t tag);
// not sure what the varint type is here
void PBDataWriterWriteBareVarint(PBDataWriter* writer, void* value, uint32_t tag);
void PBDataWriterWriteDataField(PBDataWriter* writer, NSData* value, uint32_t tag);
void PBDataWriterWriteDoubleField(PBDataWriter* writer, double value, uint32_t tag);
void PBDataWriterWriteFixed32Field(PBDataWriter* writer, int32_t value, uint32_t tag);
void PBDataWriterWriteFixed64Field(PBDataWriter* writer, int64_t value, uint32_t tag);
void PBDataWriterWriteFloatField(PBDataWriter* writer, float value, uint32_t tag);
void PBDataWriterWriteInt32Field(PBDataWriter* writer, int32_t value, uint32_t tag);
void PBDataWriterWriteInt64Field(PBDataWriter* writer, int64_t value, uint32_t tag);
void PBDataWriterWriteSfixed32Field(PBDataWriter* writer, int32_t value, uint32_t tag);
void PBDataWriterWriteSfixed64Field(PBDataWriter* writer, int64_t value, uint32_t tag);
void PBDataWriterWriteSint32Field(PBDataWriter* writer, int32_t value, uint32_t tag);
void PBDataWriterWriteSint64Field(PBDataWriter* writer, int64_t value, uint32_t tag);
// also unsure here
void PBDataWriterWriteSubgroup(PBDataWriter* writer, void* group, uint32_t tag);
void PBDataWriterWriteUint32Field(PBDataWriter* writer, uint32_t value, uint32_t tag);
void PBDataWriterWriteUint64Field(PBDataWriter* writer, uint64_t value, uint32_t tag);

// not entirely sure about that second parameter type
void PBDataWriterWriteSubmessage(PBDataWriter* writer, PBCodable* codable, uint32_t tag);
