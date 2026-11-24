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

#ifndef _GCDirectionalGamepad_H_
#define _GCDirectionalGamepad_H_

#import <GameController/GCMicroGamepad.h>

NS_ASSUME_NONNULL_BEGIN

/* Names of the elements of a directional gamepad. */
extern NSString *const GCInputDirectionalDpad;
extern NSString *const GCInputDirectionalTouchSurfaceButton;
extern NSString *const GCInputDirectionalCardinalDpad;
extern NSString *const GCInputDirectionalCenterButton;

/* The profile of a remote with a directional touch surface and no other motion input. */
@interface GCDirectionalGamepad : GCMicroGamepad
@end

NS_ASSUME_NONNULL_END

#endif
