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

// not sure what this type is
typedef int PBDataReaderMark;

@interface PBDataReader : NSObject

@end

// seems to be either a macro or an inline function
// (probably the latter)
static inline BOOL PBReaderHasMoreData(PBDataReader* reader) {
	return FALSE; // stubbed
};

static inline void PBReaderReadTag32AndType(PBDataReader* reader, uint32_t* tag, uint8_t* type) {
	return;
};

static inline BOOL PBReaderHasError(PBDataReader* reader) {
	return FALSE;
};

static inline BOOL PBReaderReadBOOL(PBDataReader* reader) {
	return FALSE;
};

static inline uint32_t PBReaderReadUint32(PBDataReader *reader) {
	return 0;
}

static inline int32_t PBReaderReadInt32(PBDataReader* reader) {
	return 0;
}

static inline uint64_t PBReaderReadUint64(PBDataReader* reader) {
	return 0;
};

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
