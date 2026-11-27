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

#import <AVFoundation/AVCaptureInput.h>
#import <CoreGraphics/CGDirectDisplay.h>
#import <CoreMedia/CMTime.h>

@interface AVCaptureScreenInput : AVCaptureInput

@property (nonatomic) CMTime minFrameDuration;
@property (nonatomic) CGRect cropRect;
@property (nonatomic) CGFloat scaleFactor;
@property (nonatomic) BOOL capturesCursor;
@property (nonatomic) BOOL capturesMouseClicks;
@property (nonatomic) BOOL removesDuplicateFrames;

- (instancetype)initWithDisplayID:(CGDirectDisplayID)displayID;

@end
