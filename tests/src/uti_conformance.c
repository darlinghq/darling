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

// UTI conformance is transitive and reflexive: public.plain-text conforms to
// public.text, which conforms to public.data. UTTypeCreatePreferredIdentifierForTag
// with a conforming-to type applies the same rule.

#include <CoreServices/CoreServices.h>
#include <stdio.h>

static int failures;

static void check(const char *label, int ok)
{
	printf("%s %s\n", ok ? "PASS" : "FAIL", label);
	if (!ok)
		failures++;
}

int main(void)
{
	check("plain-text conforms to itself", UTTypeConformsTo(CFSTR("public.plain-text"), CFSTR("public.plain-text")));
	check("plain-text conforms to public.text", UTTypeConformsTo(CFSTR("public.plain-text"), CFSTR("public.text")));
	check("plain-text conforms to public.data (transitive)", UTTypeConformsTo(CFSTR("public.plain-text"), CFSTR("public.data")));
	check("plain-text does not conform to public.image", !UTTypeConformsTo(CFSTR("public.plain-text"), CFSTR("public.image")));
	check("public.data does not conform to public.plain-text", !UTTypeConformsTo(CFSTR("public.data"), CFSTR("public.plain-text")));

	CFStringRef uti = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("txt"), CFSTR("public.data"));
	check("txt conforming to public.data resolves to public.plain-text", uti != NULL && CFEqual(uti, CFSTR("public.plain-text")));
	if (uti != NULL)
		CFRelease(uti);
	uti = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("txt"), CFSTR("public.image"));
	check("txt conforming to public.image is not found", uti == NULL);
	if (uti != NULL)
		CFRelease(uti);

	printf(failures ? "failures=%d\n" : "PASS\n", failures);
	return failures != 0;
}
