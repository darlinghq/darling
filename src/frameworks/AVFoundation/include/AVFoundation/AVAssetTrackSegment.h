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

#import <Foundation/Foundation.h>
#import <CoreMedia/CMTimeMapping.h>

// A mapping of a range of the source media to a range of track time, or an empty stretch of the track.
@interface AVAssetTrackSegment : NSObject {
@protected
    CMTimeMapping _timeMapping;
}

@property (nonatomic, readonly) CMTimeMapping timeMapping;
@property (nonatomic, readonly, getter=isEmpty) BOOL empty;

@end
