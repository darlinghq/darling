/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

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

#include <CarbonCore/TextEncodingConverter.h>
#include <unicode/ucnv.h>
#include <unicode/normalizer2.h>
#include <CarbonCore/MacErrors.h>
#include <ctype.h>
#include <string.h>

struct OpaqueTECObjectRef
{
	UConverter* inputConverter;
	UConverter* outputConverter;
	const UNormalizer2* normalizer;
	UChar buffer[4096];
	size_t bufferUsed;
};

static UConverter* createConverter(TextEncodingBase base, TextEncodingFormat format)
{
	UErrorCode error = U_ZERO_ERROR;

	switch (base)
	{
		case kTextEncodingUnicodeDefault:
		{
			const char* enc;

			switch (format)
			{
				case kUnicodeUTF16Format:
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
					enc = "UTF-16LE";
#else
					enc = "UTF-16BE";
#endif
					break;
				case kUnicodeUTF7Format:
					enc = "UTF-7";
					break;
			   	case kUnicodeUTF8Format:
					enc = "UTF-8";
					break;
			   	case kUnicodeUTF32Format:
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
					enc = "UTF-32LE";
#else
					enc = "UTF-32BE";
#endif
					break;
				case kUnicodeUTF16BEFormat:
					enc = "UTF-16BE";
					break;
			  	case kUnicodeUTF16LEFormat:
					enc = "UTF-16LE";
					break;
				case kUnicodeUTF32BEFormat:
					enc = "UTF-32BE";
					break;
				case kUnicodeUTF32LEFormat:
					enc = "UTF-32LE";
					break;
				default:
					return NULL;
			}

			return ucnv_open(enc, &error);
		}
		case kTextEncodingMacRoman:
			return ucnv_open("macintosh", &error);
		case kTextEncodingISOLatin1:
			return ucnv_open("ISO-8859-1", &error);
		case kTextEncodingWindowsLatin1:
			return ucnv_open("windows-1252", &error);
		case kTextEncodingUS_ASCII:
			return ucnv_open("US-ASCII", &error);
		default:
			return NULL;
	}
}

static void unpackEncoding(TextEncoding enc, TextEncodingBase* base, TextEncodingVariant* variant, TextEncodingFormat* format)
{
	*base = GetTextEncodingBase(enc);
	*variant = GetTextEncodingVariant(enc);
	*format = GetTextEncodingFormat(enc);
}

OSStatus TECCreateConverter(TECObjectRef *newEncodingConverter, TextEncoding inputEncoding, TextEncoding outputEncoding)
{
	TextEncodingFormat format;
	TextEncodingBase base;
	TextEncodingVariant variant;
	OpaqueTECObjectRef* obj = new OpaqueTECObjectRef;

	obj->inputConverter = obj->outputConverter = NULL;
	obj->normalizer = NULL;
	obj->bufferUsed = 0;

	unpackEncoding(inputEncoding, &base, &variant, &format);

	obj->inputConverter = createConverter(base, format);
	if (!obj->inputConverter)
	{
		TECDisposeConverter(obj);
		*newEncodingConverter = NULL;
		return unimpErr;
	}

	unpackEncoding(outputEncoding, &base, &variant, &format);

	obj->outputConverter = createConverter(base, format);
	if (!obj->outputConverter)
	{
		TECDisposeConverter(obj);
		*newEncodingConverter = NULL;
		return unimpErr;
	}

	if (base == kTextEncodingUnicodeDefault)
	{
		switch (variant)
		{
			case kUnicodeNoSubset:
				break;
			case kUnicodeNormalizationFormD:
			{
				UErrorCode error = U_ZERO_ERROR;
				obj->normalizer = unorm2_getNFDInstance(&error);
				break;
			}
			case kUnicodeNormalizationFormC:
			{
				UErrorCode error = U_ZERO_ERROR;
				obj->normalizer = unorm2_getNFCInstance(&error);
				break; 
			}
			case kUnicodeHFSPlusDecompVariant:
			case kUnicodeHFSPlusCompVariant:
			{
				UErrorCode error = U_ZERO_ERROR;
				obj->normalizer = unorm2_getNFKDInstance(&error);
				break;
			}
		}
	}

	*newEncodingConverter = obj;
	return noErr;
}

OSStatus TECConvertText(TECObjectRef encodingConverter, ConstTextPtr inputBuffer,
		ByteCount inputBufferLength, ByteCount *actualInputLength,
		TextPtr outputBuffer, ByteCount outputBufferLength, ByteCount *actualOutputLength)
{
	if (actualInputLength != NULL)
		*actualInputLength = 0;
	if (actualOutputLength != NULL)
		*actualOutputLength = 0;

	while (outputBufferLength > 0)
	{
		if (encodingConverter->bufferUsed > 0)
		{
			// flush buffer
			UErrorCode error = U_ZERO_ERROR;
			char* target = (char*) outputBuffer;
			const UChar* source = encodingConverter->buffer;
			ByteCount inputUsed;

			ucnv_fromUnicode(encodingConverter->outputConverter,
					&target, target + outputBufferLength,
					&source, source + encodingConverter->bufferUsed,
					NULL, false, &error);

			if (error != U_ZERO_ERROR && error != U_BUFFER_OVERFLOW_ERROR)
				return paramErr;

			ByteCount bytesWritten = target - ((char*)outputBuffer);
			if (actualOutputLength != NULL)
				*actualOutputLength += bytesWritten;

			inputUsed = source - encodingConverter->buffer;
			if (inputUsed < encodingConverter->bufferUsed)
			{
				memmove(encodingConverter->buffer,
						encodingConverter->buffer + inputUsed,
						(encodingConverter->bufferUsed - inputUsed) * sizeof(UChar));
			}
			encodingConverter->bufferUsed -= inputUsed;
			outputBuffer += bytesWritten;
			outputBufferLength -= bytesWritten;

			if (error == U_BUFFER_OVERFLOW_ERROR)
				break;
		}

		if (inputBufferLength <= 0)
			break;

		// Consume input
		{
			UChar* target = encodingConverter->buffer + encodingConverter->bufferUsed;
			const char* source = (const char*) inputBuffer;
			UErrorCode error = U_ZERO_ERROR;

			ucnv_toUnicode(encodingConverter->inputConverter,
					&target, encodingConverter->buffer + (sizeof(encodingConverter->buffer) / sizeof(encodingConverter->buffer[0])),
					&source, source + inputBufferLength,
					NULL, false, &error);

			if (error != U_ZERO_ERROR && error != U_BUFFER_OVERFLOW_ERROR)
				return paramErr;

			encodingConverter->bufferUsed += target - (encodingConverter->buffer + encodingConverter->bufferUsed);
			ByteCount inConsumed = source - ((const char*)inputBuffer);
			if (actualInputLength != NULL)
				*actualInputLength += inConsumed;
			inputBufferLength -= inConsumed;
			inputBuffer = (ConstTextPtr) source;
		}

		// TODO: normalize
		// Normalization may cause the data to no longer fit into our internal buffer :-/
	}

	return noErr;
}

OSStatus TECFlushText(TECObjectRef encodingConverter, TextPtr outputBuffer, ByteCount outputBufferLength, ByteCount *actualOutputLength)
{
	return TECConvertText(encodingConverter, NULL, 0, NULL, outputBuffer, outputBufferLength, actualOutputLength);
}

OSStatus TECClearConverterContextInfo(TECObjectRef conv)
{
	if (!conv)
		return paramErr;
	conv->bufferUsed = 0;
	if (conv->inputConverter)
		ucnv_reset(conv->inputConverter);
	if (conv->outputConverter)
		ucnv_reset(conv->outputConverter);
	return noErr;
}

OSStatus TECGetTextEncodingFromInternetName(TextEncoding *encoding, ConstStr255Param internetName)
{
	struct InternetName
	{
		const char* name;
		TextEncoding encoding;
	};

	static const InternetName names[] = {
		{ "utf-8", CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF8Format) },
		{ "utf-16", CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF16Format) },
		{ "utf-16be", CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF16BEFormat) },
		{ "utf-16le", CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF16LEFormat) },
		{ "utf-32", CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF32Format) },
		{ "utf-32be", CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF32BEFormat) },
		{ "utf-32le", CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF32LEFormat) },
		{ "us-ascii", kTextEncodingUS_ASCII },
		{ "ascii", kTextEncodingUS_ASCII },
		// The ISO Latin 1 names map to the Windows Latin 1 base, as on macOS.
		{ "iso-8859-1", kTextEncodingWindowsLatin1 },
		{ "latin1", kTextEncodingWindowsLatin1 },
		{ "macintosh", CreateTextEncoding(kTextEncodingMacRoman, 2, 0) },
		{ "shift_jis", CreateTextEncoding(0xa01, 1, 0) },
	};

	if (!encoding || !internetName)
		return paramErr;

	// internetName is a Pascal string: a length byte followed by the characters
	char name[256];
	size_t len = internetName[0];
	for (size_t i = 0; i < len; i++)
		name[i] = (char) tolower((unsigned char) internetName[1 + i]);
	name[len] = '\0';

	for (const InternetName& entry : names)
	{
		if (strcmp(entry.name, name) == 0)
		{
			*encoding = entry.encoding;
			return noErr;
		}
	}

	*encoding = 0xffff; // kTextEncodingUnknown
	return kTextUnsupportedEncodingErr;
}

OSStatus TECDisposeConverter(TECObjectRef conv)
{
	ucnv_close(conv->inputConverter);
	ucnv_close(conv->outputConverter);
	delete conv;
	return noErr;
}

