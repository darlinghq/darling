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

TextEncoding CreateTextEncoding(TextEncodingBase encodingBase, TextEncodingVariant encodingVariant, TextEncodingFormat encodingFormat)
{
	TextEncoding rv = encodingBase & 0xffff;
	rv |= (encodingVariant & 0xff) << 16;
	rv |= (encodingFormat & 0xff) << 24;
	return rv;
}

OSStatus UpgradeScriptInfoToTextEncoding(ScriptCode iTextScriptID, LangCode iTextLanguageID, RegionCode iTextRegionID, ConstStr255Param iTextFontname, TextEncoding *oTextEncoding)
{
	if (!oTextEncoding)
		return paramErr;

	switch (iTextScriptID)
	{
		case 0: // smRoman
			*oTextEncoding = 0;
			break;
		case 1: // smJapanese
			*oTextEncoding = 1;
			break;
		case 2: // smTradChinese
			*oTextEncoding = 2;
			break;
		case 3: // smKorean
			*oTextEncoding = 3;
			break;
		case 4: // smArabic
			*oTextEncoding = 4;
			break;
		case 5: // smHebrew
			*oTextEncoding = 5;
			break;
		case 6: // smGreek
			*oTextEncoding = 6;
			break;
		case 7: // smCyrillic
			*oTextEncoding = 7;
			break;
		case 25: // smSimpChinese
			*oTextEncoding = 25;
			break;
		case -1: // smCurrentScript
		default:
			*oTextEncoding = CreateTextEncoding(kTextEncodingUnicodeDefault, kUnicodeNoSubset, kUnicodeUTF8Format);
			break;
	}
	return noErr;
}



