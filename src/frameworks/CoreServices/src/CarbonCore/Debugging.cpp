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
#include <CarbonCore/Debugging.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "UserBreak.h"

static void defaultHandler(OSType componentSignature, UInt32 options, const char *assertionString,
	const char *exceptionLabelString, const char *errorString, const char *fileName,
	long lineNumber, ConstStr255Param outputMsg)
{
	DebugStr(outputMsg);
}

static DebugAssertOutputHandlerUPP debugAssertHandler = defaultHandler;

void DebugAssert(OSType componentSignature, UInt32 options, const char *assertionString,
	const char *exceptionLabelString, const char *errorString, const char *fileName,
	long lineNumber, void *value)
{
	Str255 out;

	std::stringstream ss;
	ss << "DebugAssert: " << assertionString << " " << exceptionLabelString
		<< " " << errorString << " [" << fileName << ":" << lineNumber << "]\n";

	std::string str = ss.str();
	out[0] = std::min<uint8_t>(str.length(), 255);
	memcpy(&out[1], str.c_str(), out[0]);

	debugAssertHandler(componentSignature, options, assertionString, exceptionLabelString, errorString, fileName, lineNumber, out);
}

void InstallDebugAssertOutputHandler(DebugAssertOutputHandlerUPP handler)
{
	if (handler == nullptr)
		debugAssertHandler = defaultHandler;
	else
		debugAssertHandler = handler;
}

UInt32 TaskLevel(void)
{
	return 0;
}

void DebugStr(ConstStr255Param msg)
{
	std::cerr.write((const char*) &msg[1], msg[0]);
	std::cerr << std::endl;
	doUserBreak();
}

namespace
{
	struct MacErrorName
	{
		int code;
		const char* names;
	};

	// Sorted by code; constants sharing a code are joined by " / ".
	const MacErrorName g_macErrorNames[] = {
#include "MacErrorNames.inc"
	};
}

const char* GetMacOSStatusErrorString(OSStatus err)
{
	const MacErrorName* begin = std::begin(g_macErrorNames);
	const MacErrorName* end = std::end(g_macErrorNames);
	const MacErrorName* it = std::lower_bound(begin, end, int(err),
		[](const MacErrorName& e, int code) { return e.code < code; });

	if (it != end && it->code == int(err))
		return it->names;
	return "";
}

// No descriptive texts are available; the documented contract is the empty string
// when no text exists for a code.
const char* GetMacOSStatusCommentString(OSStatus err)
{
	return "";
}
