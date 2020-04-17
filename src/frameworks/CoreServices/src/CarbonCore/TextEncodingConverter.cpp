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

#include <CoreServices/TextEncodingConverter.h>
#include <unicode/ucnv.h>
#include <unicode/normalizer2.h>
#include <CoreServices/MacErrors.h>

struct OpaqueTECObjectRef
{
	UConverter* inputConverter;
	UConverter* outputConverter;
	const UNormalizer2* normalizer;
	UChar buffer[64];
	size_t bufferUsed;
};

static UConverter* createConverter(TextEncodingBase base, TextEncodingFormat format)
{
	switch (base)
	{
		case kTextEncodingUnicodeDefault:
		{
			const char* enc;
			UErrorCode error = U_ZERO_ERROR;

			switch (format)
			{
				case kUnicodeUTF16Format:
					enc = "UTF-16";
					break;
				case kUnicodeUTF7Format:
					enc = "UTF-7";
					break;
			   	case kUnicodeUTF8Format:
					enc = "UTF-8";
					break;
			   	case kUnicodeUTF32Format:
					enc = "UTF-32";
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
		default:
			return NULL;
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

	format = (inputEncoding >> 16) & 0xff;
	base = (inputEncoding >> 0) & 0xff;
	variant = (inputEncoding >> 8) & 0xff;

	obj->inputConverter = createConverter(base, format);
	if (!obj->inputConverter)
	{
		TECDisposeConverter(obj);
		*newEncodingConverter = NULL;
		return unimpErr;
	}

	format = (outputEncoding >> 16) & 0xff;
	base = (outputEncoding >> 0) & 0xff;
	variant = (outputEncoding >> 8) & 0xff;

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

			*actualOutputLength = target - ((char*)outputBuffer);

			inputUsed = source - encodingConverter->buffer;
			if (inputUsed < encodingConverter->bufferUsed)
			{
				memmove(encodingConverter->buffer,
						encodingConverter->buffer + inputUsed,
						encodingConverter->bufferUsed - inputUsed);
			}
			encodingConverter->bufferUsed -= inputUsed;
			outputBuffer += *actualOutputLength;
			outputBufferLength -= *actualOutputLength;

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
			*actualInputLength += source - ((const char*)inputBuffer);
			inputBufferLength -= source - ((const char*)inputBuffer);
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

OSStatus TECDisposeConverter(TECObjectRef conv)
{
	ucnv_close(conv->inputConverter);
	ucnv_close(conv->outputConverter);
	delete conv;
	return noErr;
}

