/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

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

#ifndef _MediaPlayer_H_
#define _MediaPlayer_H_

#import <Foundation/Foundation.h>

#import <MediaPlayer/MPMediaItemArtwork.h>
#import <MediaPlayer/MPNowPlayingInfoCenter.h>
#import <MediaPlayer/MPRemoteCommandCenter.h>

FOUNDATION_EXTERN NSString * const MPMediaItemPropertyAlbumTitle;
FOUNDATION_EXTERN NSString * const MPMediaItemPropertyArtist;
FOUNDATION_EXTERN NSString * const MPMediaItemPropertyArtwork;
FOUNDATION_EXTERN NSString * const MPMediaItemPropertyPlaybackDuration;
FOUNDATION_EXTERN NSString * const MPMediaItemPropertyTitle;
FOUNDATION_EXTERN NSString * const MPNowPlayingInfoPropertyCurrentPlaybackDate;
FOUNDATION_EXTERN NSString * const MPNowPlayingInfoPropertyElapsedPlaybackTime;
FOUNDATION_EXTERN NSString * const MPNowPlayingInfoPropertyPlaybackRate;

#endif
