/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _CRYPTOTOKENKIT_TKTOKENWATCHER_H_
#define _CRYPTOTOKENKIT_TKTOKENWATCHER_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface TKTokenWatcher : NSObject
{
	// Declared here, not in the implementation: the i386 slice uses the
	// legacy Objective-C ABI, which needs every ivar in the interface.
	id _insertionHandler;
	NSMutableDictionary* _removalHandlers;
	NSMutableArray* _tokenIDs;
}

@property (readonly) NSArray<NSString *> *tokenIDs;

- (instancetype)init;
- (instancetype)initWithInsertionHandler:(void (^)(NSString *tokenID))insertionHandler;
- (void)setInsertionHandler:(void (^)(NSString *tokenID))insertionHandler;
- (void)addRemovalHandler:(void (^)(NSString *tokenID))removalHandler forTokenID:(NSString *)tokenID;

@end

NS_ASSUME_NONNULL_END

#endif
