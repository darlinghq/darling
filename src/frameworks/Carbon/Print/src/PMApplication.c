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

#ifndef __LP64__

#include <Print/PMApplication.h>

OSStatus PMSessionBeginDocument(PMPrintSession printSession, PMPrintSettings printSettings, PMPageFormat pageFormat)
{
	return PMSessionBeginCGDocumentNoDialog(printSession, printSettings, pageFormat);
}

OSStatus PMSessionEndDocument(PMPrintSession printSession)
{
	return PMSessionEndDocumentNoDialog(printSession);
}

OSStatus PMSessionBeginPage(PMPrintSession printSession, PMPageFormat pageFormat, const PMRect* pageFrame)
{
	return PMSessionBeginPageNoDialog(printSession, pageFormat, pageFrame);
}

OSStatus PMSessionEndPage(PMPrintSession printSession)
{
	return PMSessionEndPageNoDialog(printSession);
}

// There is no print panel to show yet, so the dialogs are cancelled, which is what a user does that
// dismisses them: the application does not print and keeps the objects as they were.
OSStatus PMSessionPageSetupDialog(PMPrintSession printSession, PMPageFormat pageFormat, Boolean* accepted)
{
	if (PMSessionError(printSession) == kPMInvalidPrintSession)
		return kPMInvalidPrintSession;
	if (accepted)
		*accepted = false;
	return noErr;
}

OSStatus PMSessionPrintDialog(PMPrintSession printSession, PMPrintSettings printSettings, PMPageFormat constPageFormat, Boolean* accepted)
{
	if (PMSessionError(printSession) == kPMInvalidPrintSession)
		return kPMInvalidPrintSession;
	if (accepted)
		*accepted = false;
	return noErr;
}

#endif
