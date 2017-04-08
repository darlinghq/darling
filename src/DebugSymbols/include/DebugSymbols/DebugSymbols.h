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

#include <CoreFoundation/CoreFoundation.h>

// https://github.com/apple/swift-lldb/blob/f1c2f6decd564d6530d1c57bdc5745e287f01e2a/source/Host/macosx/Symbols.cpp#L49-L50

CFURLRef DBGCopyFullDSYMURLForUUID(CFUUIDRef uuid, CFURLRef exec_url);
CFDictionaryRef DBGCopyDSYMPropertyLists(CFURLRef dsym_url);

void DBGAddDSYMURL(void);
void DBGBlockPath(void);
void DBGCopyDSYMURLForUUID(void);
void DBGCopyDSYMURLForUUIDWithOptions(void);
void DBGCopyFullDSYMURLForUUIDWithOptions(void);
void DBGCopyMatchingUUIDsForURL(void);
void DBGDwarfClose(void);
void DBGDwarfGetAttributeKey(void);
void DBGDwarfGetCompileUnitFunctions(void);
void DBGDwarfGetCompileUnitLineTable(void);
void DBGDwarfGetCompileUnits(void);
void DBGDwarfGetDIEObjectsForAddress(void);
void DBGDwarfGetFirstChild(void);
void DBGDwarfGetLinkMap(void);
void DBGDwarfGetParent(void);
void DBGDwarfGetSibling(void);
void DBGDwarfGetTagKey(void);
void DBGDwarfOpen(void);
void DBGExplicitSearchPathAdd(void);
void DBGExplicitSearchPathRemove(void);
void DBGExplicitSearchPathRemoveAll(void);
void DBGPauseWatchingPaths(void);
void DBGRegisterObserver(void);
void DBGResumeWatchingPaths(void);
void DBGSpotlightStart(void);
void DBGSpotlightStop(void);
void DBGTerminate(void);
void DBGUnregisterObserver(void);
