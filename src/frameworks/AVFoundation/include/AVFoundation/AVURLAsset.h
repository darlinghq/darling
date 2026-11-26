/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel
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

#import <AVFoundation/AVAsset.h>

@interface AVURLAsset : AVAsset

+ (NSArray<NSString *> *)audiovisualTypes;
+ (NSArray<NSString *> *)audiovisualMIMETypes;
+ (instancetype)URLAssetWithURL:(NSURL *)URL options:(NSDictionary<NSString *, id> *)options;

- (instancetype)initWithURL:(NSURL *)URL options:(NSDictionary<NSString *, id> *)options;

@property (nonatomic, readonly, copy) NSURL *URL;

@end
