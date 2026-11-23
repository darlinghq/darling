/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel
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

#import <GameController/GCControllerElement.h>
#import "GCInputInternal.h"

@implementation GCControllerElement {
	GCControllerElement *_collection;
	BOOL _analog;
	NSSet<NSString *> *_aliases;
}

- (instancetype)init
{
	return [self initWithCollection:nil isAnalog:NO aliases:[NSSet set]];
}

- (instancetype)initWithCollection:(GCControllerElement *)collection isAnalog:(BOOL)analog aliases:(NSSet<NSString *> *)aliases
{
	self = [super init];
	if (!self)
		return nil;
	_collection = collection; // un-retained weak reference
	_analog = analog;
	_aliases = [aliases copy];
	return self;
}

- (void)dealloc
{
	[_aliases release];
	[super dealloc];
}

- (GCControllerElement *)collection
{
	return _collection;
}

- (BOOL)isAnalog
{
	return _analog;
}

- (NSSet<NSString *> *)aliases
{
	return _aliases ? _aliases : [NSSet set];
}

@end
