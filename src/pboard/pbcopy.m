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


#import <Foundation/Foundation.h>
#import <AppKit/NSPasteboard.h>
#import <AppKit/NSDisplay.h>

int main(int argc, const char *argv[]) {
    [NSAutoreleasePool new];

    NSFileHandle *handle = [NSFileHandle fileHandleWithStandardInput];
    NSData *data = [handle readDataToEndOfFile];

    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard setData: data forType: NSStringPboardType];

    // While we're the clipboard owner, make sure to process incoming requests.
    NSInteger changeCount = [pasteboard changeCount];
    NSDisplay *display = [NSDisplay currentDisplay];
    while (changeCount == [pasteboard changeCount]) {
        [display nextEventMatchingMask: NSAnyEventMask
                             untilDate: [NSDate distantFuture]
                                inMode: NSDefaultRunLoopMode
                               dequeue: YES];
    }
}
