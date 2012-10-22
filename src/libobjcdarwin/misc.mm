#import <Foundation/NSException.h>
#include "visibility.h"

DARLING_VISIBLE extern "C" void objc_enumerationMutation(id obj)
{
	[NSException raise: NSGenericException format: @"Collection %@ was mutated while being enumerated", obj];
}

