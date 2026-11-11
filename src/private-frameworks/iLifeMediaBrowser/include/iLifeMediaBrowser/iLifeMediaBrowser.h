/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ILIFEMEDIABROWSER_H_
#define _ILIFEMEDIABROWSER_H_

#import <Foundation/Foundation.h>

extern NSString *const ILMediaObjectCommentsKey;

@interface ILMediaManager : NSObject
@end

@interface ILFolderMediaManager : ILMediaManager
@end

@interface ILiPhotoMediaManager : ILMediaManager
@end

@interface ILiTunesMediaManager : ILMediaManager
@end

#endif
