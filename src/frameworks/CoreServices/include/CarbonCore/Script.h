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

#ifndef _CS_SCRIPT_H
#define _CS_SCRIPT_H
#include <CoreServices/MacTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// Results GetScriptManagerVariable gives for a selector it does not know.
enum {
	smNotInstalled = 0,
	smBadVerb = -1,
	smBadScript = -2
};

// Script Manager variable selectors (deprecated).
enum {
	smVersion = 0,
	smMunged = 2,
	smEnabled = 4,
	smBidirect = 6,
	smFontForce = 8,
	smIntlForce = 10,
	smForced = 12,
	smDefault = 14,
	smPrint = 16,
	smSysScript = 18,
	smLastScript = 20,
	smKeyScript = 22,
	smSysRef = 24,
	smKeyCache = 26,
	smKeySwap = 28,
	smGenFlags = 30,
	smOverride = 32,
	smCharPortion = 34,
	smDoubleByte = 36,
	smKCHRCache = 38,
	smRegionCode = 40,
	smKeyDisableState = 42
};

// Region codes (verXxx) smRegionCode can return; the ones with a locale in the public header.
enum {
	verUS = 0,
	verFrance = 1,
	verBritain = 2,
	verGermany = 3,
	verItaly = 4,
	verNetherlands = 5,
	verFlemish = 6,
	verSweden = 7,
	verSpain = 8,
	verDenmark = 9,
	verPortugal = 10,
	verFrCanada = 11,
	verNorway = 12,
	verIsrael = 13,
	verJapan = 14,
	verAustralia = 15,
	verFinland = 17,
	verFrSwiss = 18,
	verGrSwiss = 19,
	verGreece = 20,
	verIceland = 21,
	verMalta = 22,
	verTurkey = 24,
	verItalianSwiss = 36,
	verRomania = 39,
	verLithuania = 41,
	verPoland = 42,
	verHungary = 43,
	verEstonia = 44,
	verLatvia = 45,
	verIran = 48,
	verRussia = 49,
	verKorea = 51,
	verChina = 52,
	verTaiwan = 53,
	verThailand = 54,
	verCzech = 56,
	verSlovak = 57,
	verUkraine = 62,
	verSlovenian = 66,
	verCroatia = 68,
	verBrazil = 71,
	verBulgaria = 72,
	verCatalonia = 73,
	verEngCanada = 82
};

long GetScriptManagerVariable(short selector);

#ifdef __cplusplus
}
#endif

#endif
