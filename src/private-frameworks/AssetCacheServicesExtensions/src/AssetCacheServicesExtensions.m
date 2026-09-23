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

#import <AssetCacheServicesExtensions/AssetCacheServicesExtensions.h>

// Location of the content caching metrics SQLite database, as documented in Apple's
// "Content caching metrics on Mac" (Apple Platform Deployment).
NSString *const kACMetricsDatabaseDirectory = @"/Library/Application Support/Apple/AssetCache/Metrics";
NSString *const kACMetricsDatabaseName = @"Metrics.db";

/* Darling runs no content caching service, so that database never exists. The methods Activity
   Monitor calls are not known yet (it stops in AppKit nib loading first), so none are guessed. */
@implementation AssetCacheMetricsReader
@end

@implementation AssetCacheServicesManager
@end
