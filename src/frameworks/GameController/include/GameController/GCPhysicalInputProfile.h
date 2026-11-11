/*
 This file is part of Darling.

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

#ifndef _GCPhysicalInputProfile_H_
#define _GCPhysicalInputProfile_H_

#import <Foundation/Foundation.h>
#import <GameController/GCControllerAxisInput.h>
#import <GameController/GCControllerButtonInput.h>
#import <GameController/GCControllerDirectionPad.h>

/* Apple's public documentation declares these as type aliases of the GCController* classes. */
typedef GCControllerButtonInput GCDeviceButtonInput;
typedef GCControllerAxisInput GCDeviceAxisInput;
typedef GCControllerDirectionPad GCDeviceDirectionPad;

@interface GCPhysicalInputProfile : NSObject

@property(readonly) NSDictionary<NSString *, GCDeviceButtonInput *> *buttons;
@property(readonly) NSDictionary<NSString *, GCDeviceAxisInput *> *axes;
@property(readonly) NSDictionary<NSString *, GCDeviceDirectionPad *> *dpads;

@end

#endif
