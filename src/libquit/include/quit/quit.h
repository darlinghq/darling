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


#ifndef _quit_H_
#define _quit_H_

void* LQCachePort(void);
void* LQForceQuit(void);
void* LQNotifyCpuResource(void);
void* LQNotifyFacelessQuitHappening(void);
void* LQNotifyQuitLikely(void);
void* LQNotifySpinLikely(void);
void* LQNotifyStuckApp(void);
void* LQNotifyUiQuitHappening(void);
void* LQNotifyUnstuckApp(void);
void* LQNotifyWakeupsResource(void);
void* LQReportCpuResource(void);
void* LQReportPotentialHang(void);
void* LQReportPotentialSpin(void);
void* LQReportStuckApp(void);
void* LQReportUnstuckApp(void);
void* LQReportWakeupsResource(void);

#endif
