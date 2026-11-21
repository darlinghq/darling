/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// One record of a .DS_Store: a setting Finder keeps for one item of the folder (or for "." itself)
@interface FinderDSStoreRecord : NSObject
@property (readonly) NSString *name;    // the item's name, as stored
@property (readonly) FourCharCode code; // what the record is about, e.g. 'Iloc'
@property (readonly) FourCharCode type; // 'long' 'shor' 'bool' 'blob' 'type' 'ustr' 'comp' 'dutc'
// An NSNumber ('long' 'shor' 'bool' 'comp' 'dutc'), NSData ('blob') or NSString ('ustr', and 'type' as its four
// characters)
@property (readonly) id value;
@end

// The records of a .DS_Store file, the settings Finder keeps for a folder's window and items. A file that is malformed
// in any way is rejected as a whole.
@interface FinderDSStore : NSObject
// nil when the data is not a well-formed store, or is larger than 16 MB
+ (nullable instancetype)storeWithData:(NSData *)data;
// Reads <folder>/.DS_Store; nil when there is none or it can't be read
+ (nullable instancetype)storeForFolder:(NSURL *)folder;
@property (readonly) NSArray<FinderDSStoreRecord *> *records; // in the store's order
// The record with this code for the item with this name. Names are compared case-sensitively, by canonical
// equivalence (a precomposed and a decomposed name are the same).
- (nullable FinderDSStoreRecord *)recordForName:(NSString *)name code:(FourCharCode)code;
@end

NS_ASSUME_NONNULL_END
