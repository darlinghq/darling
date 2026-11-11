// Copyright (C) 2026 Lubos Dolezel
//
// CreateObjSpecifier builds an 'obj ' record holding the desired class, the
// container, the key form and the key data, and disposes its inputs on request.
// The program passes unchanged on macOS 27.0.1 (arm64 and x86_64).
#include <CoreServices/CoreServices.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	AEDesc key, container, specifier;
	SInt32 index = 3;
	CHECK(AECreateDesc(typeSInt32, &index, sizeof(index), &key) == noErr);
	CHECK(AECreateDesc(typeNull, NULL, 0, &container) == noErr);

	CHECK(CreateObjSpecifier('docu', &container, formAbsolutePosition, &key, true, &specifier) == noErr);
	CHECK(specifier.descriptorType == typeObjectSpecifier);
	CHECK(key.descriptorType == typeNull); // disposed

	long count = 0;
	CHECK(AECountItems(&specifier, &count) == noErr);
	CHECK(count == 4);

	DescType type, desiredClass = 0, form = 0;
	Size size;
	CHECK(AEGetParamPtr(&specifier, keyAEDesiredClass, typeType, &type, &desiredClass, sizeof(desiredClass), &size) == noErr);
	CHECK(desiredClass == 'docu');
	AEDesc from;
	CHECK(AEGetParamDesc(&specifier, keyAEContainer, typeWildCard, &from) == noErr);
	CHECK(from.descriptorType == typeNull);
	AEDisposeDesc(&from);
	CHECK(AEGetParamPtr(&specifier, keyAEKeyForm, typeEnumerated, &type, &form, sizeof(form), &size) == noErr);
	CHECK(form == formAbsolutePosition);
	SInt32 value = 0;
	CHECK(AEGetParamPtr(&specifier, keyAEKeyData, typeSInt32, &type, &value, sizeof(value), &size) == noErr);
	CHECK(value == 3);
	AEDisposeDesc(&specifier);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
