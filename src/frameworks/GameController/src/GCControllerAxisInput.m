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

#import <GameController/GCControllerAxisInput.h>
#import "GCInputInternal.h"
#import <math.h>

@implementation GCControllerAxisInput {
	float _value;
	GCControllerAxisValueChangedHandler _valueChangedHandler;
}

@synthesize valueChangedHandler = _valueChangedHandler;

- (void)dealloc
{
	[_valueChangedHandler release];
	[super dealloc];
}

- (float)value
{
	return _value;
}

- (void)setValue:(float)value
{
	_value = fmaxf(-1.0f, fminf(1.0f, value));
}

- (void)_setValue:(float)value handlerQueue:(dispatch_queue_t)queue profileHandler:(void (^)(GCControllerElement *))profileHandler
{
	float clamped = fmaxf(-1.0f, fminf(1.0f, value));
	if (clamped == _value)
		return;

	_value = clamped;

	GCControllerAxisValueChangedHandler vh = [_valueChangedHandler copy];
	void (^phCopy)(GCControllerElement *) = [profileHandler copy];

	dispatch_queue_t targetQueue = queue ? queue : dispatch_get_main_queue();
	dispatch_async(targetQueue, ^{
		if (vh)
			vh(self, clamped);
		if (phCopy)
			phCopy(self);

		[vh release];
		[phCopy release];
	});
}

@end
