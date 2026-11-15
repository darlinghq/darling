/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel
 Copyright (C) 2026 Darling developers

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

#ifndef _USERNOTIFICATIONS_UNNOTIFICATIONSOUND_H_
#define _USERNOTIFICATIONS_UNNOTIFICATIONSOUND_H_

#import <Foundation/Foundation.h>

typedef NSString *UNNotificationSoundName;

@interface UNNotificationSound : NSObject <NSCopying, NSSecureCoding> {
    NSString *_soundName;
    float _audioVolume;
    BOOL _critical;
}

+ (instancetype)defaultSound;
+ (instancetype)defaultCriticalSound;
+ (instancetype)defaultCriticalSoundWithAudioVolume:(float)volume;
+ (instancetype)soundNamed:(UNNotificationSoundName)name;
+ (instancetype)criticalSoundNamed:(UNNotificationSoundName)name;
+ (instancetype)criticalSoundNamed:(UNNotificationSoundName)name withAudioVolume:(float)volume;

@property(readonly, copy) NSString *soundName;
@property(readonly) float audioVolume;
@property(readonly, getter=isCritical) BOOL critical;

@end

#endif
