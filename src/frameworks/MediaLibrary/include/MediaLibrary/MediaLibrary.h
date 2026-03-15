/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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


#ifndef _MediaLibrary_H_
#define _MediaLibrary_H_

#import <Foundation/Foundation.h>

#import <MediaLibrary/MLMediaLibraryProtocol.h>
#import <MediaLibrary/MLSCacheDeleteProtocol.h>
#import <MediaLibrary/MLSMediaLibraryProtocol.h>
#import <MediaLibrary/MLMediaLibrary.h>
#import <MediaLibrary/MLMediaLibraryImpl.h>
#import <MediaLibrary/MLMediaGroup.h>
#import <MediaLibrary/MLMediaGroupImpl.h>
#import <MediaLibrary/MLMediaObject.h>
#import <MediaLibrary/MLMediaObjectImpl.h>
#import <MediaLibrary/MLMediaSource.h>
#import <MediaLibrary/MLMediaSourceImpl.h>
#import <MediaLibrary/MLPMR.h>
#import <MediaLibrary/MLPMRContextImpl.h>
#import <MediaLibrary/MLPMRContext.h>

void* MLLog(void);
void* MLLogWithLevel(void);
void* MLPMRInstrument_private_createPool(void);
void* MLPMRInstrument_private_deletePool(void);
void* _Z16ensurePathExistsP8NSString(void);
void* _Z26mlpmrContext_cppInstrumentPK10__CFString(void);
void* _ZN22MLPMRInstrumentFactory16createInstrumentEP12MLPMRContextPK10__CFStringS4_(void);
void* ilmbpmr_getCurrentTime(void);
void* mlpmrContext_applicationName(void);
void* mlpmrContext_instrument(void);
void* mlpmrContext_logPath(void);
void* mlpmrContext_setParameters(void);
void* mlpmrFini(void);
void* mlpmrInit(void);
void* mlpmrInstrument_logBool(void);
void* mlpmrInstrument_logDouble(void);
void* mlpmrInstrument_logSInt32(void);
void* mlpmrInstrument_logSInt64(void);
void* mlpmrInstrument_logString(void);
void* mlpmrInstrument_logUInt32(void);
void* mlpmrInstrument_logUInt64(void);
void* mlpmrTimer_begin(void);
void* mlpmrTimer_delta(void);
void* mlpmrTimer_end(void);
void* mlpmrTimer_init(void);
void* mlpmrTimer_pause(void);
void* mlpmrTimer_resume(void);

#endif
