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


#ifndef _ForceFeedback_H_
#define _ForceFeedback_H_

void* FFCreateDevice(void);
void* FFDeviceCreateEffect(void);
void* FFDeviceEscape(void);
void* FFDeviceGetForceFeedbackCapabilities(void);
void* FFDeviceGetForceFeedbackProperty(void);
void* FFDeviceGetForceFeedbackState(void);
void* FFDeviceReleaseEffect(void);
void* FFDeviceSendForceFeedbackCommand(void);
void* FFDeviceSetCooperativeLevel(void);
void* FFDeviceSetForceFeedbackProperty(void);
void* FFEffectDownload(void);
void* FFEffectEscape(void);
void* FFEffectGetEffectStatus(void);
void* FFEffectGetParameters(void);
void* FFEffectSetParameters(void);
void* FFEffectStart(void);
void* FFEffectStop(void);
void* FFEffectUnload(void);
void* FFIsForceFeedback(void);
void* FFReleaseDevice(void);

#endif
