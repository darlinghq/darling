/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#ifndef _MobileGestalt_H_
#define _MobileGestalt_H_

void* MGCancelNotifications(void);
void* MGCopyAnswer(void);
void* MGCopyAnswerWithError(void);
void* MGCopyMultipleAnswers(void);
void* MGGetBoolAnswer(void);
void* MGGetFloat32Answer(void);
void* MGGetProductType(void);
void* MGGetSInt32Answer(void);
void* MGGetSInt64Answer(void);
void* MGGetStringAnswer(void);
void* MGIsDeviceOfType(void);
void* MGIsDeviceOneOfType(void);
void* MGIsQuestionValid(void);
void* MGRegisterForBulkUpdates(void);
void* MGRegisterForUpdates(void);
void* MGSetAnswer(void);
void* MGSetLogHandler(void);
void* _MGCacheCopyValueForKey(void);
void* _MGCacheValid(void);
void* _MGClearInProcessCache(void);
void* _MGCopyDeviceDescription(void);
void* _MGCopyIteratedDeviceDescription(void);
void* _MGGetFastPathLog(void);
void* _MGIterateDevices(void);
void* _MGLog(void);
void* _MGPrintIteratedDeviceDescription(void);
void* _MGRebuildCache(void);
void* _MGServerCopyAnswerWithError(void);
void* _MGServerQuestionIsPlatform(void);
void* _MGSetServer(void);
void* _MGWaitForDevices(void);
void* _MGWriteCache(void);

#endif
