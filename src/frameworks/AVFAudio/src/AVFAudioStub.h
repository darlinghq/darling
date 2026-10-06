/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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

#ifndef AVFAUDIO_STUB_H
#define AVFAUDIO_STUB_H

#import <Foundation/Foundation.h>
#import <objc/runtime.h>

/*
 Most AVAudio classes here are placeholders that forward rather than implement, and they used
 to answer methodSignatureForSelector: with a hardcoded "v@:" for every selector. "v@:" is the
 encoding of a method taking no arguments, so the runtime rejected any call that passed
 arguments with NSForwardSignatureError - "invoked with 3 args, but 2 expected" for
 -[AVAudioPlayerNode setVolume:], 4-vs-2 for -[AVAudioFile initForReading:error:]. That happens
 before forwardInvocation: runs, so a client cannot get past it.

 The argument count of an Objective-C method is not a guess: it is the number of colons in the
 selector. Counting them gives the arity the runtime checks, so a stub can advertise the right
 number of arguments for any selector without anyone enumerating methods.

 Types are all reported as object pointers, which is wrong for scalar parameters (a float volume,
 an options mask). That is safe *here* and only here because these classes forward without ever
 reading the argument buffer: forwardInvocation: logs the selector and returns. Anything that
 starts using an incoming argument must declare that selector properly instead of relying on
 this, because the values would be misread.
 */
static inline NSMethodSignature* AVAudioStubSignature(SEL selector)
{
	// One colon per argument, plus a generous upper bound so the buffer cannot overflow on a
	// selector nobody sane writes; the encoding is rebuilt to fit either way.
	char encoding[64];
	const char* name = sel_getName(selector);
	unsigned colons = 0;

	if (name != NULL)
	{
		for (const char* p = name; *p != '\0'; p++)
		{
			if (*p == ':')
			{
				colons++;
			}
		}
	}

	if (colons + 4 > sizeof(encoding))
	{
		colons = (unsigned) sizeof(encoding) - 4;
	}

	// "v@:" is void return, self, _cmd; then one object pointer per argument.
	memcpy(encoding, "v@:", 3);
	memset(encoding + 3, '@', colons);
	encoding[3 + colons] = '\0';

	return [NSMethodSignature signatureWithObjCTypes:encoding];
}

#endif