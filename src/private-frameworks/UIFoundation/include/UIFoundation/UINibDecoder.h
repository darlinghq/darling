/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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

#include <Foundation/Foundation.h>

@class UINibDecoder;

// Delegate of a UINibDecoder. The selectors are the ones of NSKeyedUnarchiverDelegate, so a
// delegate written for a keyed unarchiver (NSNib is one) works unchanged.
@protocol UINibDecoderDelegate <NSObject>
@optional
// Called with every object decoded through its class (not with strings, data and numbers);
// the returned object, when not nil, replaces it.
- (id) unarchiver: (UINibDecoder *) decoder didDecodeObject: (id) object;
- (void) unarchiver: (UINibDecoder *) decoder willReplaceObject: (id) object withObject: (id) replacement;
@end

// Reads the binary "NIBArchive" format that ibtool writes for nibs and storyboards.
// Keyed coding only: object graphs are decoded with -decodeObjectForKey: and friends, the
// way NSKeyedUnarchiver would decode them; the unkeyed NSCoder methods raise
// NSInvalidArchiveOperationException. Every archived object is decoded once; later
// references return the same instance.
struct UINibDecoderState;

@interface UINibDecoder : NSCoder {
@private
    // Private state; declared here because the i386 legacy ABI has no @implementation ivars.
    struct UINibDecoderState *_s;
    id<UINibDecoderDelegate> _delegate;
}

// Returns nil if the data is not a well-formed NIBArchive (bad header, tables outside the
// data, indexes out of range, unknown value types).
- (instancetype) initForReadingWithData: (NSData *) data;

@property (assign) id<UINibDecoderDelegate> delegate;

// Decodes objects archived under className as instances of cls instead.
- (void) setClass: (Class) cls forClassName: (NSString *) className;
- (Class) classForClassName: (NSString *) className;

// Makes later references to object resolve to replacement (as NSKeyedUnarchiver does for
// its private -replaceObject:withObject:, which nib loading code relies on).
- (void) replaceObject: (id) object withObject: (id) replacement;

@end
