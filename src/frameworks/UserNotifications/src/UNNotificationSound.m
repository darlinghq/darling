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

#import <UserNotifications/UNNotificationSound.h>

@implementation UNNotificationSound

@synthesize soundName = _soundName;
@synthesize audioVolume = _audioVolume;
@synthesize critical = _critical;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

+ (instancetype)defaultSound
{
    return [self soundNamed:nil];
}

+ (instancetype)defaultCriticalSound
{
    return [self criticalSoundNamed:nil withAudioVolume:1.0f];
}

+ (instancetype)defaultCriticalSoundWithAudioVolume:(float)volume
{
    return [self criticalSoundNamed:nil withAudioVolume:volume];
}

+ (instancetype)soundNamed:(UNNotificationSoundName)name
{
    UNNotificationSound *sound = [[[self alloc] init] autorelease];
    sound->_soundName = [name copy];
    sound->_audioVolume = 1.0f;
    sound->_critical = NO;
    return sound;
}

+ (instancetype)criticalSoundNamed:(UNNotificationSoundName)name
{
    return [self criticalSoundNamed:name withAudioVolume:1.0f];
}

+ (instancetype)criticalSoundNamed:(UNNotificationSoundName)name withAudioVolume:(float)volume
{
    UNNotificationSound *sound = [[[self alloc] init] autorelease];
    sound->_soundName = [name copy];
    sound->_audioVolume = volume;
    sound->_critical = YES;
    return sound;
}

- (void)dealloc
{
    [_soundName release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    UNNotificationSound *copy = [[[self class] allocWithZone:zone] init];
    copy->_soundName = [self.soundName copyWithZone:zone];
    copy->_audioVolume = self.audioVolume;
    copy->_critical = self.critical;
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_soundName forKey:@"soundName"];
    [coder encodeFloat:_audioVolume forKey:@"audioVolume"];
    [coder encodeBool:_critical forKey:@"critical"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    if (self) {
        _soundName = [[coder decodeObjectOfClass:[NSString class] forKey:@"soundName"] copy];
        _audioVolume = [coder decodeFloatForKey:@"audioVolume"];
        _critical = [coder decodeBoolForKey:@"critical"];
    }
    return self;
}

@end
