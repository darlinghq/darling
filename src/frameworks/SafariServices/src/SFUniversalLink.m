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

#import <SafariServices/SFUniversalLink.h>

/* Universal links map a web domain to an app through the associated-domains
 * registrations of installed apps, which Darling has no store for. Every URL is
 * therefore reported as having no handling app, which is the documented answer
 * for a URL that is not a universal link. */

@implementation SFUniversalLink

@synthesize webpageURL = _webpageURL;
@synthesize applicationURL = _applicationURL;
@synthesize enabled = _enabled;

- (instancetype)initWithWebpageURL:(NSURL *)url
{
	[self release];
	return nil;
}

- (void)dealloc
{
	[_webpageURL release];
	[_applicationURL release];
	[super dealloc];
}

@end
