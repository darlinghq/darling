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

// A mark remembers the end of the enclosing message while a nested message is read.
typedef struct {
	size_t previousEnd;
	size_t end;
} PBDataReaderMark;

// Reads the Protocol Buffers wire format (https://protobuf.dev/programming-guides/encoding/).
@interface PBDataReader : NSObject {
@public
	NSData *_data;
	const uint8_t *_bytes;
	size_t _position;
	size_t _end;
	BOOL _error;
}

- (instancetype)initWithData:(NSData*)data;

@property (readonly) NSData *data;

@end

BOOL PBReaderHasMoreData(PBDataReader* reader);
void PBReaderReadTag32AndType(PBDataReader* reader, uint32_t* tag, uint8_t* type);
BOOL PBReaderHasError(PBDataReader* reader);
BOOL PBReaderReadBOOL(PBDataReader* reader);
uint32_t PBReaderReadUint32(PBDataReader *reader);
int32_t PBReaderReadInt32(PBDataReader* reader);
uint64_t PBReaderReadUint64(PBDataReader* reader);
int64_t PBReaderReadInt64(PBDataReader* reader);
double PBReaderReadDouble(PBDataReader* reader);

NSString* PBReaderReadString(PBDataReader* reader);
NSData* PBReaderReadData(PBDataReader* reader);
BOOL PBReaderPlaceMark(PBDataReader* reader, PBDataReaderMark* mark);
void PBReaderRecallMark(PBDataReader* reader, PBDataReaderMark* mark);
BOOL PBReaderSkipValueWithTag(PBDataReader* reader, uint32_t tag, uint8_t type);

// guesses
uint16_t PBReaderReadBigEndianFixed16(PBDataReader* reader);
uint32_t PBReaderReadBigEndianFixed32(PBDataReader* reader);
uint64_t PBReaderReadBigEndianFixed64(PBDataReader* reader);
void* PBReaderReadVarIntBuf(PBDataReader* reader);
