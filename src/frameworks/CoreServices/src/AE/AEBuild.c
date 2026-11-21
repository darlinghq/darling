/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// AEBuildDesc, AEBuildParameters and AEBuildAppleEvent: descriptors and Apple Events described by a
// format string, built on top of the descriptor functions of AEDescriptors.c.
//
// The format string describes a descriptor as one of:
//   an integer                     a 16-bit signed integer descriptor when it fits (-32767...32767), else a
//                                  32-bit one (larger values wrap)
//   "text"                         a text descriptor (\" and \\ escape the quote and backslash)
//   'abcd' or abcd                 an enumerated descriptor holding that four-character code (padded with spaces
//                                  or cut to four characters)
//   [ d, d, ... ]                  a list
//   { key: d, key: d, ... }        a record; keys are four-character codes, quoted or not
//   code { key: d, ... }           a record whose descriptor type is code
//   code ( data )                  a descriptor of type code; data is an integer (coerced to the type),
//                                  "text" or $hex digits$ (the raw bytes), a four-character code (its value),
//                                  or @ for an argument from the argument list
//   null()                         the null descriptor
//   @                              a descriptor taken from the argument list (copied)
// The arguments substituted for @ inside code(...) depend on the type: SInt16/UInt16 and Boolean take an int,
// SInt32 and UInt32 a long, SInt64 and UInt64 a long long, the floating-point types a double, type, enumerated and
// keyword a four-character code, and 'TEXT' a C string; every other type takes a long size followed by a pointer to
// that many bytes.

#include <AE/AE.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct AEBuildParser {
	const char* start;
	const char* cur;
	va_list* args;
	AEBuildErrorCode code;
	UInt32 position;
} AEBuildParser;

static OSStatus syntaxError(AEBuildParser* parser, AEBuildErrorCode code, const char* at)
{
	if (parser->code == aeBuildSyntaxNoErr) {
		parser->code = code;
		parser->position = (UInt32) (at - parser->start);
	}
	return errAEBuildSyntaxError;
}

static void skipSpace(AEBuildParser* parser)
{
	while (*parser->cur == ' ' || *parser->cur == '\t' || *parser->cur == '\n' || *parser->cur == '\r')
		parser->cur++;
}

static Boolean isIdentStart(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static Boolean isIdentPart(char c)
{
	return isIdentStart(c) || (c >= '0' && c <= '9');
}

static FourCharCode packCode(const char* text, size_t length)
{
	FourCharCode code = 0;
	for (size_t i = 0; i < 4; i++)
		code = (code << 8) | (i < length ? (unsigned char) text[i] : ' ');
	return code;
}

// Reads 'abcd' or a bare word as a four-character code.
static OSStatus parseCode(AEBuildParser* parser, FourCharCode* code)
{
	const char* begin = parser->cur;
	if (*begin == '\'') {
		const char* end = strchr(begin + 1, '\'');
		if (end == NULL)
			return syntaxError(parser, aeBuildSyntaxMissingQuote, begin + strlen(begin));
		*code = packCode(begin + 1, end - begin - 1);
		parser->cur = end + 1;
		return noErr;
	}
	if (!isIdentStart(*begin))
		return syntaxError(parser, aeBuildSyntaxBadToken, begin);
	const char* end = begin;
	while (isIdentPart(*end))
		end++;
	*code = packCode(begin, end - begin);
	parser->cur = end;
	return noErr;
}

static int hexValue(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

// Reads $hex digits$ into a malloc'ed buffer.
static OSStatus parseHex(AEBuildParser* parser, uint8_t** bytes, Size* size)
{
	const char* begin = ++parser->cur;
	size_t digits = 0;
	while (hexValue(parser->cur[digits]) >= 0)
		digits++;
	const char* end = parser->cur + digits;
	if (*end != '$') {
		if (*end == 0)
			return syntaxError(parser, aeBuildSyntaxNoCloseHex, end);
		return syntaxError(parser, aeBuildSyntaxBadHex, end);
	}
	if (digits & 1)
		return syntaxError(parser, aeBuildSyntaxOddHex, end);
	*bytes = malloc(digits / 2 + 1);
	if (*bytes == NULL)
		return memFullErr;
	for (size_t i = 0; i < digits / 2; i++)
		(*bytes)[i] = (uint8_t) (hexValue(begin[2 * i]) << 4 | hexValue(begin[2 * i + 1]));
	*size = (Size) (digits / 2);
	parser->cur = end + 1;
	return noErr;
}

// Reads a quoted string into a malloc'ed buffer (no terminator).
static OSStatus parseString(AEBuildParser* parser, uint8_t** bytes, Size* size)
{
	const char* p = parser->cur + 1;
	size_t length = 0;
	uint8_t* out = malloc(strlen(p) + 1);
	if (out == NULL)
		return memFullErr;
	while (*p != '"') {
		if (*p == 0) {
			free(out);
			return syntaxError(parser, aeBuildSyntaxBadDesc, p);
		}
		if (*p == '\\' && p[1] != 0)
			p++;
		out[length++] = (uint8_t) *p++;
	}
	*bytes = out;
	*size = (Size) length;
	parser->cur = p + 1;
	return noErr;
}

// Reads an optionally negative decimal integer, wrapping to 64 bits.
static Boolean parseInteger(AEBuildParser* parser, int64_t* value)
{
	const char* p = parser->cur;
	Boolean negative = false;
	if (*p == '-') {
		negative = true;
		p++;
	}
	if (*p < '0' || *p > '9')
		return false;
	uint64_t magnitude = 0;
	while (*p >= '0' && *p <= '9')
		magnitude = magnitude * 10 + (uint64_t) (*p++ - '0');
	*value = (int64_t) (negative ? 0 - magnitude : magnitude);
	parser->cur = p;
	return true;
}

static OSStatus parseDesc(AEBuildParser* parser, AEDesc* result, int depth);

// The datum inside code( ... ), as a descriptor of the given type.
static OSStatus parseTypedData(AEBuildParser* parser, DescType type, AEDesc* result)
{
	OSStatus status = noErr;
	uint8_t* bytes = NULL;
	Size size = 0;
	skipSpace(parser);
	char c = *parser->cur;

	if (c == ')') {
		status = AECreateDesc(type, NULL, 0, result);
	} else if (c == '-' || (c >= '0' && c <= '9')) {
		int64_t value;
		if (!parseInteger(parser, &value))
			return syntaxError(parser, aeBuildSyntaxBadData, parser->cur);
		SInt32 asLong = (SInt32) value;
		status = type == typeSInt32 ? AECreateDesc(type, &asLong, sizeof(asLong), result)
			: AECoercePtr(typeSInt32, &asLong, sizeof(asLong), type, result);
	} else if (c == '"') {
		if ((status = parseString(parser, &bytes, &size)) == noErr)
			status = AECreateDesc(type, bytes, size, result);
	} else if (c == '$') {
		if ((status = parseHex(parser, &bytes, &size)) == noErr)
			status = AECreateDesc(type, bytes, size, result);
	} else if (isIdentStart(c)) {
		FourCharCode code;
		if ((status = parseCode(parser, &code)) == noErr)
			status = AECreateDesc(type, &code, sizeof(code), result);
	} else if (c == '@') {
		parser->cur++;
		va_list* args = parser->args;
		switch (type) {
			case typeSInt16: case typeUInt16: {
				SInt16 value = (SInt16) va_arg(*args, int);
				status = AECreateDesc(type, &value, sizeof(value), result);
				break;
			}
			case typeBoolean: {
				Boolean value = (Boolean) va_arg(*args, int);
				status = AECreateDesc(type, &value, sizeof(value), result);
				break;
			}
			case typeSInt32: case typeUInt32: {
				SInt32 value = (SInt32) va_arg(*args, long);
				status = AECreateDesc(type, &value, sizeof(value), result);
				break;
			}
			case typeSInt64: case typeUInt64: {
				SInt64 value = va_arg(*args, long long);
				status = AECreateDesc(type, &value, sizeof(value), result);
				break;
			}
			case typeIEEE32BitFloatingPoint: {
				Float32 value = (Float32) va_arg(*args, double);
				status = AECreateDesc(type, &value, sizeof(value), result);
				break;
			}
			case typeIEEE64BitFloatingPoint: {
				Float64 value = va_arg(*args, double);
				status = AECreateDesc(type, &value, sizeof(value), result);
				break;
			}
			case typeType: case typeEnumerated: case typeKeyword: {
				FourCharCode value = va_arg(*args, FourCharCode);
				status = AECreateDesc(type, &value, sizeof(value), result);
				break;
			}
			case typeChar: {
				const char* text = va_arg(*args, const char*);
				status = AECreateDesc(type, text, text ? (Size) strlen(text) : 0, result);
				break;
			}
			default: {
				long length = va_arg(*args, long);
				const void* data = va_arg(*args, const void*);
				status = AECreateDesc(type, data, length, result);
				break;
			}
		}
	} else {
		return syntaxError(parser, aeBuildSyntaxBadData, parser->cur);
	}
	free(bytes);
	if (status != noErr) {
		AEInitializeDesc(result);
		return status;
	}

	skipSpace(parser);
	if (*parser->cur != ')') {
		AEDisposeDesc(result);
		return syntaxError(parser, aeBuildSyntaxNoCloseParen, parser->cur + (*parser->cur ? 1 : 0));
	}
	parser->cur++;
	return noErr;
}

// key: desc pairs up to (not including) the closing character; used for records and for the parameters
// of an Apple Event, where the list runs to the end of the string.
static OSStatus parseKeyedItems(AEBuildParser* parser, AERecord* record, char closing, int depth)
{
	skipSpace(parser);
	if (*parser->cur == closing && closing != 0)
		return noErr;
	if (*parser->cur == 0 && closing == 0)
		return noErr;
	for (;;) {
		skipSpace(parser);
		FourCharCode key;
		if (*parser->cur != '\'' && !isIdentStart(*parser->cur))
			return syntaxError(parser, aeBuildSyntaxNoKey, parser->cur + (*parser->cur ? 1 : 0));
		OSStatus status = parseCode(parser, &key);
		if (status != noErr)
			return status;
		skipSpace(parser);
		if (*parser->cur != ':')
			return syntaxError(parser, aeBuildSyntaxNoColon, parser->cur + (*parser->cur ? 1 : 0));
		parser->cur++;
		AEDesc value;
		if ((status = parseDesc(parser, &value, depth + 1)) != noErr)
			return status;
		status = AEPutParamDesc(record, key, &value);
		AEDisposeDesc(&value);
		if (status != noErr)
			return status;
		skipSpace(parser);
		if (*parser->cur == ',') {
			parser->cur++;
			continue;
		}
		if (closing != 0 && *parser->cur != closing)
			return syntaxError(parser, aeBuildSyntaxNoCloseBrace, parser->cur + (*parser->cur ? 1 : 0));
		return noErr;
	}
}

static OSStatus parseRecordBody(AEBuildParser* parser, DescType type, AEDesc* result, int depth)
{
	parser->cur++; // {
	AERecord record;
	OSStatus status = AECreateList(NULL, 0, true, &record);
	if (status != noErr)
		return status;
	if ((status = parseKeyedItems(parser, &record, '}', depth)) != noErr) {
		AEDisposeDesc(&record);
		return status;
	}
	parser->cur++; // }
	record.descriptorType = type;
	*result = record;
	return noErr;
}

static OSStatus parseListBody(AEBuildParser* parser, AEDesc* result, int depth)
{
	parser->cur++; // [
	AEDescList list;
	OSStatus status = AECreateList(NULL, 0, false, &list);
	if (status != noErr)
		return status;
	skipSpace(parser);
	if (*parser->cur != ']') {
		for (;;) {
			AEDesc item;
			if ((status = parseDesc(parser, &item, depth + 1)) != noErr)
				break;
			status = AEPutDesc(&list, 0, &item);
			AEDisposeDesc(&item);
			if (status != noErr)
				break;
			skipSpace(parser);
			if (*parser->cur == ',') {
				parser->cur++;
				continue;
			}
			if (*parser->cur != ']')
				status = syntaxError(parser, aeBuildSyntaxNoCloseBracket, parser->cur + (*parser->cur ? 1 : 0));
			break;
		}
	}
	if (status != noErr) {
		AEDisposeDesc(&list);
		return status;
	}
	parser->cur++; // ]
	*result = list;
	return noErr;
}

static OSStatus parseDesc(AEBuildParser* parser, AEDesc* result, int depth)
{
	AEInitializeDesc(result);
	if (depth > 256)
		return syntaxError(parser, aeBuildSyntaxBadDesc, parser->cur);
	skipSpace(parser);
	char c = *parser->cur;

	if (c == '-' || (c >= '0' && c <= '9')) {
		int64_t value;
		const char* at = parser->cur;
		if (!parseInteger(parser, &value))
			return syntaxError(parser, aeBuildSyntaxBadDesc, at);
		if (value > -32768 && value <= 32767) {
			SInt16 asShort = (SInt16) value;
			return AECreateDesc(typeSInt16, &asShort, sizeof(asShort), result);
		}
		SInt32 asLong = (SInt32) value;
		return AECreateDesc(typeSInt32, &asLong, sizeof(asLong), result);
	}
	if (c == '"') {
		uint8_t* bytes;
		Size size;
		OSStatus status = parseString(parser, &bytes, &size);
		if (status != noErr)
			return status;
		status = AECreateDesc(typeChar, bytes, size, result);
		free(bytes);
		return status;
	}
	if (c == '[')
		return parseListBody(parser, result, depth);
	if (c == '{')
		return parseRecordBody(parser, typeAERecord, result, depth);
	if (c == '$') {
		uint8_t* bytes;
		Size size;
		OSStatus status = parseHex(parser, &bytes, &size);
		if (status == noErr) {
			free(bytes);
			return syntaxError(parser, aeBuildSyntaxUncoercedHex, parser->cur);
		}
		return status;
	}
	if (c == '@') {
		parser->cur++;
		if (*parser->cur == '@')
			return syntaxError(parser, aeBuildSyntaxUncoercedDoubleAt, parser->cur);
		const AEDesc* source = va_arg(*parser->args, const AEDesc*);
		return AEDuplicateDesc(source, result);
	}
	if (c == '\'' || isIdentStart(c)) {
		FourCharCode code;
		OSStatus status = parseCode(parser, &code);
		if (status != noErr)
			return status;
		if (*parser->cur == '(') {
			parser->cur++;
			return parseTypedData(parser, code, result);
		}
		if (*parser->cur == '{')
			return parseRecordBody(parser, code, result, depth);
		if (*parser->cur == '[')
			return syntaxError(parser, aeBuildSyntaxCoercedList, parser->cur + 1);
		return AECreateDesc(typeEnumerated, &code, sizeof(code), result);
	}
	return syntaxError(parser, c == 0 ? aeBuildSyntaxBadDesc : aeBuildSyntaxBadToken, parser->cur);
}

static void initParser(AEBuildParser* parser, const char* format, va_list* args)
{
	parser->start = parser->cur = format;
	parser->args = args;
	parser->code = aeBuildSyntaxNoErr;
	parser->position = 0;
}

static void reportError(const AEBuildParser* parser, AEBuildError* error)
{
	if (error != NULL) {
		error->fError = parser->code;
		error->fErrorPos = parser->position;
	}
}

OSStatus vAEBuildDesc(AEDesc* dst, AEBuildError* error, const char* src, va_list args)
{
	if (dst == NULL || src == NULL)
		return paramErr;
	va_list copy;
	va_copy(copy, args);
	AEBuildParser parser;
	initParser(&parser, src, &copy);
	AEDesc built;
	OSStatus status = parseDesc(&parser, &built, 0);
	if (status == noErr) {
		skipSpace(&parser);
		if (*parser.cur != 0) {
			AEDisposeDesc(&built);
			status = syntaxError(&parser, aeBuildSyntaxNoEOF, parser.cur);
		}
	}
	va_end(copy);
	reportError(&parser, error);
	if (status == noErr)
		*dst = built;
	else
		AEInitializeDesc(dst);
	return status;
}

OSStatus AEBuildDesc(AEDesc* dst, AEBuildError* error, const char* src, ...)
{
	va_list args;
	va_start(args, src);
	OSStatus status = vAEBuildDesc(dst, error, src, args);
	va_end(args);
	return status;
}

OSStatus vAEBuildParameters(AppleEvent* event, AEBuildError* error, const char* format, va_list args)
{
	if (event == NULL || format == NULL)
		return paramErr;
	va_list copy;
	va_copy(copy, args);
	AEBuildParser parser;
	initParser(&parser, format, &copy);
	OSStatus status = parseKeyedItems(&parser, event, 0, 0);
	if (status == noErr) {
		skipSpace(&parser);
		if (*parser.cur != 0)
			status = syntaxError(&parser, aeBuildSyntaxNoEOF, parser.cur);
	}
	va_end(copy);
	reportError(&parser, error);
	return status;
}

OSStatus AEBuildParameters(AppleEvent* event, AEBuildError* error, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	OSStatus status = vAEBuildParameters(event, error, format, args);
	va_end(args);
	return status;
}

OSStatus vAEBuildAppleEvent(AEEventClass theClass, AEEventID theID, DescType addressType, const void* addressData,
	long addressLength, SInt16 returnID, SInt32 transactionID, AppleEvent* result, AEBuildError* error,
	const char* paramsFmt, va_list args)
{
	if (result == NULL || paramsFmt == NULL)
		return paramErr;
	AEDesc address;
	OSStatus status = AECreateDesc(addressType, addressData, addressLength, &address);
	if (status != noErr)
		return status;
	AppleEvent event;
	status = AECreateAppleEvent(theClass, theID, &address, returnID, transactionID, &event);
	AEDisposeDesc(&address);
	if (status != noErr)
		return status;
	if ((status = vAEBuildParameters(&event, error, paramsFmt, args)) != noErr) {
		AEDisposeDesc(&event);
		AEInitializeDesc(result);
		return status;
	}
	*result = event;
	return noErr;
}

OSStatus AEBuildAppleEvent(AEEventClass theClass, AEEventID theID, DescType addressType, const void* addressData,
	long addressLength, SInt16 returnID, SInt32 transactionID, AppleEvent* result, AEBuildError* error,
	const char* paramsFmt, ...)
{
	va_list args;
	va_start(args, paramsFmt);
	OSStatus status = vAEBuildAppleEvent(theClass, theID, addressType, addressData, addressLength, returnID,
		transactionID, result, error, paramsFmt, args);
	va_end(args);
	return status;
}
