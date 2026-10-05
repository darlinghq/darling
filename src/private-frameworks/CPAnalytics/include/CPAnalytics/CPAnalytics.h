/*
  This file is part of Darling.

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

/*
   Apple's telemetry framework. Photo Booth links it and binds five symbols: this class,
   two payload keys, and two functions.

   There is no analytics backend here and no telemetry should be sent anywhere, so every entry
   is the neutral answer. The keys exist so callers can build a payload dictionary without
   crashing; the two functions do nothing at all, which is the correct behaviour for a view
   appearing in a build that collects nothing.
*/

extern NSString *const CPAnalyticsPayloadClassNameKey;
extern NSString *const CPAnalyticsPayloadViewIDKey;

void CPAnalyticsEventViewDidAppear(id view, id payload);
void CPAnalyticsEventViewDidDisappear(id view, id payload);

@interface CPAnalytics : NSObject

+ (instancetype) sharedInstance;

@end