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


#ifndef _InstantMessage_H_
#define _InstantMessage_H_

#import <Foundation/Foundation.h>

#import <InstantMessage/IMAuxVideoProvider.h>
#import <InstantMessage/IMAuxVideoProvider.h>
#import <InstantMessage/IMSharedPixelBufferPool.h>
#import <InstantMessage/_IMVideoDataSource.h>
#import <InstantMessage/IMAVManagerImpl.h>
#import <InstantMessage/IMAVManager.h>
#import <InstantMessage/IMAVControlBarImpl.h>
#import <InstantMessage/IMAVControlBar.h>
#import <InstantMessage/_IMAVControlAuxiliary.h>
#import <InstantMessage/_IMAVButtonAuxiliary.h>
#import <InstantMessage/_IMAVSliderAuxiliary.h>
#import <InstantMessage/IMAVControl.h>
#import <InstantMessage/IMAVButton.h>
#import <InstantMessage/IMAVSlider.h>
#import <InstantMessage/IMService.h>
#import <InstantMessage/iChatTheaterXPC.h>

void* IMComparePersonStatus(void);
void* IMServiceAgentClass(void);
void* IMServiceImplClass(void);

#endif
