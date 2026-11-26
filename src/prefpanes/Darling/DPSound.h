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
#import <Cocoa/Cocoa.h>

// The Sound section of the pane, built on the CoreAudio HAL (AudioObject properties) and AudioQueue:
//  - the output and input devices (name, manufacturer, channels, sample rate, transport), kept up to
//    date through a listener on kAudioHardwarePropertyDevices;
//  - the selected row of each list is the default output or input device, and selecting another row
//    makes that device the default;
//  - the volume and mute of the default output device, where the device has them;
//  - an output test (a short tone, the left channel and then the right one) on the selected output, and
//    an input test with a live level meter on the selected input.
@interface DPSoundController : NSObject <NSTableViewDataSource, NSTableViewDelegate>

// The section's view, width as given; the controller keeps it.
- (NSView *)viewWithWidth:(CGFloat)width;

// The section is on show (or leaves the screen): the listeners are installed and the lists reloaded, or the
// tests stopped and the listeners removed.
- (void)activate;
- (void)deactivate;

// Stops the tone and the input test, if running.
- (void)stopTests;

@end

// A horizontal bar showing a level between 0 and 1 (the peak amplitude); the bar is on a decibel scale.
@interface DPLevelView : NSView
@property (nonatomic) double doubleValue;
@end
