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
					enc = "UTF-8";
					break;
			}

			return ucnv_open(enc, &error);
		}
		case kTextEncodingMacRoman:
			return ucnv_open("macintosh", &error);
		case kTextEncodingISOLatin1:
			return ucnv_open("ISO-8859-1", &error);
		case kTextEncodingUS_ASCII:
			return ucnv_open("US-ASCII", &error);
		default:
			return NULL;
	}
}

static void unpackEncoding(TextEncoding enc, TextEncodingBase* base, TextEncodingVariant* variant, TextEncodingFormat* format)
{
	*format = (enc >> 24) & 0xff;
	*variant = (enc >> 16) & 0xff;
	*base = enc & 0xffff;

	// Support old Darling layout (format at >> 16, variant at >> 8)
	if (*format == 0 && ((enc >> 16) & 0xff) != 0 && *base < 0x100)
	{
		*format = (enc >> 16) & 0xff;
		*variant = (enc >> 8) & 0xff;
		*base = enc & 0xff;
	}
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
	if (!encoding || !internetName)
		return paramErr;

	char name[256] = {0};
	size_t len = 0;

	// Check if pascal string (internetName[0] is length, remaining are ASCII chars)
	unsigned char pLen = internetName[0];
	if (pLen > 0 && pLen < 128)
	{
		bool looksPascal = true;
		for (size_t i = 1; i <= pLen; ++i)
		{
			if (internetName[i] == 0)
			{
				looksPascal = false;
				break;
			}
		}
		if (looksPascal)
		{
			len = pLen;
			memcpy(name, &internetName[1], len);
		}
	}

	if (len == 0)
	{
		// Fallback to C-string
		len = strlen((const char*)internetName);
		if (len >= sizeof(name))
			len = sizeof(name) - 1;
		memcpy(name, internetName, len);
	}
	name[len] = '\0';

	// Case-insensitive check
	for (size_t i = 0; i < len; ++i)
		name[i] = (char)tolower((unsigned char)name[i]);

	if (strcmp(name, "utf-8") == 0 || strcmp(name, "utf8") == 0)
	{
		*encoding = CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF8Format);
		return noErr;
	}
	if (strcmp(name, "utf-16") == 0 || strcmp(name, "utf16") == 0)
	{
		*encoding = CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF16Format);
		return noErr;
	}
	if (strcmp(name, "utf-16be") == 0)
	{
		*encoding = CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF16BEFormat);
		return noErr;
	}
	if (strcmp(name, "utf-16le") == 0)
	{
		*encoding = CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF16LEFormat);
		return noErr;
	}
	if (strcmp(name, "utf-32") == 0 || strcmp(name, "utf32") == 0)
	{
		*encoding = CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF32Format);
		return noErr;
	}
	if (strcmp(name, "utf-32be") == 0)
	{
		*encoding = CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF32BEFormat);
		return noErr;
	}
	if (strcmp(name, "utf-32le") == 0)
	{
		*encoding = CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF32LEFormat);
		return noErr;
	}
	if (strcmp(name, "utf-7") == 0)
	{
		*encoding = CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF7Format);
		return noErr;
	}
	if (strcmp(name, "us-ascii") == 0 || strcmp(name, "ascii") == 0)
	{
		*encoding = CreateTextEncoding(kTextEncodingUS_ASCII, kTextEncodingDefaultVariant, kTextEncodingDefaultFormat);
		return noErr;
	}
	if (strcmp(name, "iso-8859-1") == 0 || strcmp(name, "latin1") == 0 || strcmp(name, "iso_8859-1") == 0)
	{
		*encoding = CreateTextEncoding(kTextEncodingISOLatin1, kTextEncodingDefaultVariant, kTextEncodingDefaultFormat);
		return noErr;
	}
	if (strcmp(name, "macintosh") == 0 || strcmp(name, "macroman") == 0 || strcmp(name, "mac-roman") == 0)
	{
		*encoding = CreateTextEncoding(kTextEncodingMacRoman, kTextEncodingDefaultVariant, kTextEncodingDefaultFormat);
		return noErr;
	}

	return kTECNoConversionPathErr;
}

OSStatus TECDisposeConverter(TECObjectRef conv)
{
	ucnv_close(conv->inputConverter);
	ucnv_close(conv->outputConverter);
	delete conv;
	return noErr;
}

