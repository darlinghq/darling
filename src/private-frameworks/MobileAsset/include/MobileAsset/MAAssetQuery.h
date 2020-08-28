/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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

#import <MobileAsset/MAAsset.h>

// seems to be implicitly depended on by `OTATrustUtilities.m` in Security
#import <MobileAsset/ASAssetQuery.h>

// really not sure if this is an enum
typedef NS_ENUM(NSInteger, MAQueryResult) {
	MAQuerySuccessful = 0,
};

@interface MAAssetQuery : NSObject

// guessing it's readonly, since it wouldn't really make sense for clients to be able to
// write results to a query
@property (readonly) NSArray<MAAsset*>* results;

@end
