/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#include <JavaNativeFoundation/JavaNativeFoundation.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* JNFAssertionFailure(void)
{
    if (verbose) puts("STUB: JNFAssertionFailure called");
    return NULL;
}

void* JNFAssertionFailureBreak(void)
{
    if (verbose) puts("STUB: JNFAssertionFailureBreak called");
    return NULL;
}

void* JNFCFToJavaBoolean(void)
{
    if (verbose) puts("STUB: JNFCFToJavaBoolean called");
    return NULL;
}

void* JNFCallBooleanMethod(void)
{
    if (verbose) puts("STUB: JNFCallBooleanMethod called");
    return NULL;
}

void* JNFCallByteMethod(void)
{
    if (verbose) puts("STUB: JNFCallByteMethod called");
    return NULL;
}

void* JNFCallCharMethod(void)
{
    if (verbose) puts("STUB: JNFCallCharMethod called");
    return NULL;
}

void* JNFCallDoubleMethod(void)
{
    if (verbose) puts("STUB: JNFCallDoubleMethod called");
    return NULL;
}

void* JNFCallFloatMethod(void)
{
    if (verbose) puts("STUB: JNFCallFloatMethod called");
    return NULL;
}

void* JNFCallIntMethod(void)
{
    if (verbose) puts("STUB: JNFCallIntMethod called");
    return NULL;
}

void* JNFCallLongMethod(void)
{
    if (verbose) puts("STUB: JNFCallLongMethod called");
    return NULL;
}

void* JNFCallObjectMethod(void)
{
    if (verbose) puts("STUB: JNFCallObjectMethod called");
    return NULL;
}

void* JNFCallShortMethod(void)
{
    if (verbose) puts("STUB: JNFCallShortMethod called");
    return NULL;
}

void* JNFCallStaticBooleanMethod(void)
{
    if (verbose) puts("STUB: JNFCallStaticBooleanMethod called");
    return NULL;
}

void* JNFCallStaticByteMethod(void)
{
    if (verbose) puts("STUB: JNFCallStaticByteMethod called");
    return NULL;
}

void* JNFCallStaticCharMethod(void)
{
    if (verbose) puts("STUB: JNFCallStaticCharMethod called");
    return NULL;
}

void* JNFCallStaticDoubleMethod(void)
{
    if (verbose) puts("STUB: JNFCallStaticDoubleMethod called");
    return NULL;
}

void* JNFCallStaticFloatMethod(void)
{
    if (verbose) puts("STUB: JNFCallStaticFloatMethod called");
    return NULL;
}

void* JNFCallStaticIntMethod(void)
{
    if (verbose) puts("STUB: JNFCallStaticIntMethod called");
    return NULL;
}

void* JNFCallStaticLongMethod(void)
{
    if (verbose) puts("STUB: JNFCallStaticLongMethod called");
    return NULL;
}

void* JNFCallStaticObjectMethod(void)
{
    if (verbose) puts("STUB: JNFCallStaticObjectMethod called");
    return NULL;
}

void* JNFCallStaticShortMethod(void)
{
    if (verbose) puts("STUB: JNFCallStaticShortMethod called");
    return NULL;
}

void* JNFCallStaticVoidMethod(void)
{
    if (verbose) puts("STUB: JNFCallStaticVoidMethod called");
    return NULL;
}

void* JNFCallVoidMethod(void)
{
    if (verbose) puts("STUB: JNFCallVoidMethod called");
    return NULL;
}

void* JNFDebugWarning(void)
{
    if (verbose) puts("STUB: JNFDebugWarning called");
    return NULL;
}

void* JNFDebugWarningBreak(void)
{
    if (verbose) puts("STUB: JNFDebugWarningBreak called");
    return NULL;
}

void* JNFDeleteGlobalRef(void)
{
    if (verbose) puts("STUB: JNFDeleteGlobalRef called");
    return NULL;
}

void* JNFDeleteWeakGlobalRef(void)
{
    if (verbose) puts("STUB: JNFDeleteWeakGlobalRef called");
    return NULL;
}

void* JNFDumpJavaObject(void)
{
    if (verbose) puts("STUB: JNFDumpJavaObject called");
    return NULL;
}

void* JNFDumpJavaStack(void)
{
    if (verbose) puts("STUB: JNFDumpJavaStack called");
    return NULL;
}

void* JNFGetBooleanField(void)
{
    if (verbose) puts("STUB: JNFGetBooleanField called");
    return NULL;
}

void* JNFGetByteField(void)
{
    if (verbose) puts("STUB: JNFGetByteField called");
    return NULL;
}

void* JNFGetCharField(void)
{
    if (verbose) puts("STUB: JNFGetCharField called");
    return NULL;
}

void* JNFGetDoubleField(void)
{
    if (verbose) puts("STUB: JNFGetDoubleField called");
    return NULL;
}

void* JNFGetFloatField(void)
{
    if (verbose) puts("STUB: JNFGetFloatField called");
    return NULL;
}

void* JNFGetIntField(void)
{
    if (verbose) puts("STUB: JNFGetIntField called");
    return NULL;
}

void* JNFGetLongField(void)
{
    if (verbose) puts("STUB: JNFGetLongField called");
    return NULL;
}

void* JNFGetObjectField(void)
{
    if (verbose) puts("STUB: JNFGetObjectField called");
    return NULL;
}

void* JNFGetShortField(void)
{
    if (verbose) puts("STUB: JNFGetShortField called");
    return NULL;
}

void* JNFGetStackTraceAsNSString(void)
{
    if (verbose) puts("STUB: JNFGetStackTraceAsNSString called");
    return NULL;
}

void* JNFGetStaticBooleanField(void)
{
    if (verbose) puts("STUB: JNFGetStaticBooleanField called");
    return NULL;
}

void* JNFGetStaticByteField(void)
{
    if (verbose) puts("STUB: JNFGetStaticByteField called");
    return NULL;
}

void* JNFGetStaticCharField(void)
{
    if (verbose) puts("STUB: JNFGetStaticCharField called");
    return NULL;
}

void* JNFGetStaticDoubleField(void)
{
    if (verbose) puts("STUB: JNFGetStaticDoubleField called");
    return NULL;
}

void* JNFGetStaticFloatField(void)
{
    if (verbose) puts("STUB: JNFGetStaticFloatField called");
    return NULL;
}

void* JNFGetStaticIntField(void)
{
    if (verbose) puts("STUB: JNFGetStaticIntField called");
    return NULL;
}

void* JNFGetStaticLongField(void)
{
    if (verbose) puts("STUB: JNFGetStaticLongField called");
    return NULL;
}

void* JNFGetStaticObjectField(void)
{
    if (verbose) puts("STUB: JNFGetStaticObjectField called");
    return NULL;
}

void* JNFGetStaticShortField(void)
{
    if (verbose) puts("STUB: JNFGetStaticShortField called");
    return NULL;
}

void* JNFGetStringUTF16UniChars(void)
{
    if (verbose) puts("STUB: JNFGetStringUTF16UniChars called");
    return NULL;
}

void* JNFGetStringUTF8Chars(void)
{
    if (verbose) puts("STUB: JNFGetStringUTF8Chars called");
    return NULL;
}

void* JNFIsInstanceOf(void)
{
    if (verbose) puts("STUB: JNFIsInstanceOf called");
    return NULL;
}

void* JNFJavaMillisToNSTimeInterval(void)
{
    if (verbose) puts("STUB: JNFJavaMillisToNSTimeInterval called");
    return NULL;
}

void* JNFJavaStackTrace(void)
{
    if (verbose) puts("STUB: JNFJavaStackTrace called");
    return NULL;
}

void* JNFJavaToCFBoolean(void)
{
    if (verbose) puts("STUB: JNFJavaToCFBoolean called");
    return NULL;
}

void* JNFJavaToNSDate(void)
{
    if (verbose) puts("STUB: JNFJavaToNSDate called");
    return NULL;
}

void* JNFJavaToNSNumber(void)
{
    if (verbose) puts("STUB: JNFJavaToNSNumber called");
    return NULL;
}

void* JNFJavaToNSString(void)
{
    if (verbose) puts("STUB: JNFJavaToNSString called");
    return NULL;
}

void* JNFNSTimeIntervalToJavaMillis(void)
{
    if (verbose) puts("STUB: JNFNSTimeIntervalToJavaMillis called");
    return NULL;
}

void* JNFNSToJavaCalendar(void)
{
    if (verbose) puts("STUB: JNFNSToJavaCalendar called");
    return NULL;
}

void* JNFNSToJavaNumber(void)
{
    if (verbose) puts("STUB: JNFNSToJavaNumber called");
    return NULL;
}

void* JNFNSToJavaString(void)
{
    if (verbose) puts("STUB: JNFNSToJavaString called");
    return NULL;
}

void* JNFNativeMethodEnter(void)
{
    if (verbose) puts("STUB: JNFNativeMethodEnter called");
    return NULL;
}

void* JNFNativeMethodExit(void)
{
    if (verbose) puts("STUB: JNFNativeMethodExit called");
    return NULL;
}

void* JNFNewBooleanArray(void)
{
    if (verbose) puts("STUB: JNFNewBooleanArray called");
    return NULL;
}

void* JNFNewByteArray(void)
{
    if (verbose) puts("STUB: JNFNewByteArray called");
    return NULL;
}

void* JNFNewCharArray(void)
{
    if (verbose) puts("STUB: JNFNewCharArray called");
    return NULL;
}

void* JNFNewDoubleArray(void)
{
    if (verbose) puts("STUB: JNFNewDoubleArray called");
    return NULL;
}

void* JNFNewFloatArray(void)
{
    if (verbose) puts("STUB: JNFNewFloatArray called");
    return NULL;
}

void* JNFNewGlobalRef(void)
{
    if (verbose) puts("STUB: JNFNewGlobalRef called");
    return NULL;
}

void* JNFNewIntArray(void)
{
    if (verbose) puts("STUB: JNFNewIntArray called");
    return NULL;
}

void* JNFNewLongArray(void)
{
    if (verbose) puts("STUB: JNFNewLongArray called");
    return NULL;
}

void* JNFNewObject(void)
{
    if (verbose) puts("STUB: JNFNewObject called");
    return NULL;
}

void* JNFNewObjectArray(void)
{
    if (verbose) puts("STUB: JNFNewObjectArray called");
    return NULL;
}

void* JNFNewShortArray(void)
{
    if (verbose) puts("STUB: JNFNewShortArray called");
    return NULL;
}

void* JNFNewWeakGlobalRef(void)
{
    if (verbose) puts("STUB: JNFNewWeakGlobalRef called");
    return NULL;
}

void* JNFNormalizedJavaStringForPath(void)
{
    if (verbose) puts("STUB: JNFNormalizedJavaStringForPath called");
    return NULL;
}

void* JNFNormalizedNSStringForPath(void)
{
    if (verbose) puts("STUB: JNFNormalizedNSStringForPath called");
    return NULL;
}

void* JNFObjectClassName(void)
{
    if (verbose) puts("STUB: JNFObjectClassName called");
    return NULL;
}

void* JNFObjectEquals(void)
{
    if (verbose) puts("STUB: JNFObjectEquals called");
    return NULL;
}

void* JNFObjectToString(void)
{
    if (verbose) puts("STUB: JNFObjectToString called");
    return NULL;
}

void* JNFObtainEnv(void)
{
    if (verbose) puts("STUB: JNFObtainEnv called");
    return NULL;
}

void* JNFPerformEnvBlock(void)
{
    if (verbose) puts("STUB: JNFPerformEnvBlock called");
    return NULL;
}

void* JNFReleaseEnv(void)
{
    if (verbose) puts("STUB: JNFReleaseEnv called");
    return NULL;
}

void* JNFReleaseStringUTF16UniChars(void)
{
    if (verbose) puts("STUB: JNFReleaseStringUTF16UniChars called");
    return NULL;
}

void* JNFReleaseStringUTF8Chars(void)
{
    if (verbose) puts("STUB: JNFReleaseStringUTF8Chars called");
    return NULL;
}

void* JNFSetBooleanField(void)
{
    if (verbose) puts("STUB: JNFSetBooleanField called");
    return NULL;
}

void* JNFSetByteField(void)
{
    if (verbose) puts("STUB: JNFSetByteField called");
    return NULL;
}

void* JNFSetCharField(void)
{
    if (verbose) puts("STUB: JNFSetCharField called");
    return NULL;
}

void* JNFSetDoubleField(void)
{
    if (verbose) puts("STUB: JNFSetDoubleField called");
    return NULL;
}

void* JNFSetFloatField(void)
{
    if (verbose) puts("STUB: JNFSetFloatField called");
    return NULL;
}

void* JNFSetIntField(void)
{
    if (verbose) puts("STUB: JNFSetIntField called");
    return NULL;
}

void* JNFSetLongField(void)
{
    if (verbose) puts("STUB: JNFSetLongField called");
    return NULL;
}

void* JNFSetObjectField(void)
{
    if (verbose) puts("STUB: JNFSetObjectField called");
    return NULL;
}

void* JNFSetShortField(void)
{
    if (verbose) puts("STUB: JNFSetShortField called");
    return NULL;
}

void* JNFSetStaticBooleanField(void)
{
    if (verbose) puts("STUB: JNFSetStaticBooleanField called");
    return NULL;
}

void* JNFSetStaticByteField(void)
{
    if (verbose) puts("STUB: JNFSetStaticByteField called");
    return NULL;
}

void* JNFSetStaticCharField(void)
{
    if (verbose) puts("STUB: JNFSetStaticCharField called");
    return NULL;
}

void* JNFSetStaticDoubleField(void)
{
    if (verbose) puts("STUB: JNFSetStaticDoubleField called");
    return NULL;
}

void* JNFSetStaticFloatField(void)
{
    if (verbose) puts("STUB: JNFSetStaticFloatField called");
    return NULL;
}

void* JNFSetStaticIntField(void)
{
    if (verbose) puts("STUB: JNFSetStaticIntField called");
    return NULL;
}

void* JNFSetStaticLongField(void)
{
    if (verbose) puts("STUB: JNFSetStaticLongField called");
    return NULL;
}

void* JNFSetStaticObjectField(void)
{
    if (verbose) puts("STUB: JNFSetStaticObjectField called");
    return NULL;
}

void* JNFSetStaticShortField(void)
{
    if (verbose) puts("STUB: JNFSetStaticShortField called");
    return NULL;
}

void* JNFSetTracePS(void)
{
    if (verbose) puts("STUB: JNFSetTracePS called");
    return NULL;
}
