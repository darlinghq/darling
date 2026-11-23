/*
 This file is part of Darling.

 Copyright (C) 2026 Darling developers

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

#import <NaturalLanguage/NaturalLanguage.h>

// The documentation gives no string values; BCP 47 "en" and "und" are used for the two languages, which
// is what macOS 26.7.3 and 10.14.4 report (black-box observation). Test: tests/src/naturallanguage_placeholder.m
NLLanguage const NLLanguageEnglish = @"en";
NLLanguage const NLLanguageUndetermined = @"und";

// The documentation gives no string values; the scheme values follow Darling's own NSLinguisticTagScheme
// values for the corresponding schemes, and match what macOS 26.7.3 and 10.14.4 report (black-box
// observation). Test: tests/src/naturallanguage_placeholder.m
NLTagScheme const NLTagSchemeLexicalClass = @"LexicalClass";
NLTagScheme const NLTagSchemeNameTypeOrLexicalClass = @"NameTypeOrLexicalClass";

// The documentation gives no string values; the tag values follow Darling's own NSLinguisticTag values
// for the corresponding tags, and match what macOS 26.7.3 and 10.14.4 report (black-box observation).
// Test: tests/src/naturallanguage_placeholder.m
NLTag const NLTagNoun = @"Noun";
NLTag const NLTagVerb = @"Verb";
NLTag const NLTagAdjective = @"Adjective";
NLTag const NLTagNumber = @"Number";
NLTag const NLTagPersonalName = @"PersonalName";
NLTag const NLTagOrganizationName = @"OrganizationName";
NLTag const NLTagPlaceName = @"PlaceName";
