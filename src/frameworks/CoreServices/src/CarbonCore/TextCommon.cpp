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

#include <CarbonCore/TextCommon.h>
#include <CarbonCore/MacErrors.h>

// TextEncoding layout (TextCommon.h): base in bits 0-15, variant in bits 16-25
// (10 bits), format in bits 26-29 (4 bits).
TextEncodingBase GetTextEncodingBase(TextEncoding encoding)
{
	return encoding & 0xffff;
}

TextEncodingVariant GetTextEncodingVariant(TextEncoding encoding)
{
	return (encoding >> 16) & 0x3ff;
}

TextEncodingFormat GetTextEncodingFormat(TextEncoding encoding)
{
	return (encoding >> 26) & 0xf;
}

TextEncoding CreateTextEncoding(TextEncodingBase encodingBase, TextEncodingVariant encodingVariant, TextEncodingFormat encodingFormat)
{
	TextEncoding rv = encodingBase & 0xffff;
	rv |= (encodingVariant & 0x3ff) << 16;
	rv |= (encodingFormat & 0xf) << 26;
	return rv;
}

OSStatus UpgradeScriptInfoToTextEncoding(ScriptCode iTextScriptID, LangCode iTextLanguageID, RegionCode iTextRegionID, ConstStr255Param iTextFontname, TextEncoding *oTextEncoding)
{
	enum { smCurrentScript = -2, smSystemScript = -1 };

	if (!oTextEncoding)
		return paramErr;

	switch (iTextScriptID)
	{
		case 0: // smRoman, as MacRoman variant 2
			*oTextEncoding = CreateTextEncoding(0, 2, 0);
			break;
		case 1: // smJapanese
		case 2: // smTradChinese
		case 3: // smKorean
		case 4: // smArabic
		case 5: // smHebrew
			*oTextEncoding = iTextScriptID;
			break;
		case 6: // smGreek
			*oTextEncoding = CreateTextEncoding(6, 2, 0);
			break;
		case 7: // smCyrillic
			*oTextEncoding = CreateTextEncoding(7, 3, 0);
			break;
		case 25: // smSimpChinese
			*oTextEncoding = 25;
			break;
		case smCurrentScript:
		case smSystemScript:
			*oTextEncoding = CreateTextEncoding(0, 2, 0);
			break;
		default:
			return paramErr;
	}
	return noErr;
}
