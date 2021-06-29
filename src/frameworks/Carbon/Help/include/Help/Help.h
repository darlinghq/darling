/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#ifndef _Help_H_
#define _Help_H_

void* AHGotoApplicationHelp(void);
void* AHGotoMainTOC(void);
void* AHGotoPage(void);
void* AHLookupAnchor(void);
void* AHRegisterHelpBook(void);
void* AHRegisterHelpBookWithURL(void);
void* AHSearch(void);
void* AppendEncodingHint(void);
void* CFStringToCString(void);
void* CopyBookDetails(void);
void* CreateBookDetailsFromAppBundle(void);
void* CreateBookDetailsFromAppBundleAtIndex(void);
void* CreateBookDetailsWithBookNameAndID(void);
void* EscapeMacCharacters(void);
void* MacPathToURLPath(void);

#endif
