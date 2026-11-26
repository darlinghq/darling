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

#ifndef _Print_PMApplication_H_
#define _Print_PMApplication_H_

#include <ApplicationServices/ApplicationServices.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __LP64__
// The calls of a document that show the progress of the printing; they make the document in the
// session like the calls of PrintCore that have no dialog
OSStatus PMSessionBeginDocument(PMPrintSession printSession, PMPrintSettings printSettings, PMPageFormat pageFormat);
OSStatus PMSessionEndDocument(PMPrintSession printSession);
OSStatus PMSessionBeginPage(PMPrintSession printSession, PMPageFormat pageFormat, const PMRect* pageFrame);
OSStatus PMSessionEndPage(PMPrintSession printSession);

// The dialogs of the page format and of the print settings. *accepted is false when the user cancelled.
OSStatus PMSessionPageSetupDialog(PMPrintSession printSession, PMPageFormat pageFormat, Boolean* accepted);
OSStatus PMSessionPrintDialog(PMPrintSession printSession, PMPrintSettings printSettings, PMPageFormat constPageFormat, Boolean* accepted);
#endif

#ifdef __cplusplus
}
#endif

#endif
