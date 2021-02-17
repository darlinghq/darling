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


#ifndef _JavaNativeFoundation_H_
#define _JavaNativeFoundation_H_

#import <Foundation/Foundation.h>

#import <JavaNativeFoundation/JNFTypeCoercion.h>
#import <JavaNativeFoundation/JNFException.h>
#import <JavaNativeFoundation/JNFTypeCoercer.h>
#import <JavaNativeFoundation/JNFInternalJavaClassToCoersionHolder.h>
#import <JavaNativeFoundation/JNFStringCoercion.h>
#import <JavaNativeFoundation/JNFNumberCoercion.h>
#import <JavaNativeFoundation/JNFDateCoercion.h>
#import <JavaNativeFoundation/JNFMapCoercion.h>
#import <JavaNativeFoundation/JNFListCoercion.h>
#import <JavaNativeFoundation/JNFSetCoercion.h>
#import <JavaNativeFoundation/JNFDefaultCoercions.h>
#import <JavaNativeFoundation/JNFJObjectWrapper.h>
#import <JavaNativeFoundation/JNFWeakJObjectWrapper.h>
#import <JavaNativeFoundation/JNFRunLoop.h>
#import <JavaNativeFoundation/JNFRunnableWrapper.h>
#import <JavaNativeFoundation/JNFRunnable.h>

void* JNFAssertionFailure(void);
void* JNFAssertionFailureBreak(void);
void* JNFCFToJavaBoolean(void);
void* JNFCallBooleanMethod(void);
void* JNFCallByteMethod(void);
void* JNFCallCharMethod(void);
void* JNFCallDoubleMethod(void);
void* JNFCallFloatMethod(void);
void* JNFCallIntMethod(void);
void* JNFCallLongMethod(void);
void* JNFCallObjectMethod(void);
void* JNFCallShortMethod(void);
void* JNFCallStaticBooleanMethod(void);
void* JNFCallStaticByteMethod(void);
void* JNFCallStaticCharMethod(void);
void* JNFCallStaticDoubleMethod(void);
void* JNFCallStaticFloatMethod(void);
void* JNFCallStaticIntMethod(void);
void* JNFCallStaticLongMethod(void);
void* JNFCallStaticObjectMethod(void);
void* JNFCallStaticShortMethod(void);
void* JNFCallStaticVoidMethod(void);
void* JNFCallVoidMethod(void);
void* JNFDebugWarning(void);
void* JNFDebugWarningBreak(void);
void* JNFDeleteGlobalRef(void);
void* JNFDeleteWeakGlobalRef(void);
void* JNFDumpJavaObject(void);
void* JNFDumpJavaStack(void);
void* JNFGetBooleanField(void);
void* JNFGetByteField(void);
void* JNFGetCharField(void);
void* JNFGetDoubleField(void);
void* JNFGetFloatField(void);
void* JNFGetIntField(void);
void* JNFGetLongField(void);
void* JNFGetObjectField(void);
void* JNFGetShortField(void);
void* JNFGetStackTraceAsNSString(void);
void* JNFGetStaticBooleanField(void);
void* JNFGetStaticByteField(void);
void* JNFGetStaticCharField(void);
void* JNFGetStaticDoubleField(void);
void* JNFGetStaticFloatField(void);
void* JNFGetStaticIntField(void);
void* JNFGetStaticLongField(void);
void* JNFGetStaticObjectField(void);
void* JNFGetStaticShortField(void);
void* JNFGetStringUTF16UniChars(void);
void* JNFGetStringUTF8Chars(void);
void* JNFIsInstanceOf(void);
void* JNFJavaMillisToNSTimeInterval(void);
void* JNFJavaStackTrace(void);
void* JNFJavaToCFBoolean(void);
void* JNFJavaToNSDate(void);
void* JNFJavaToNSNumber(void);
void* JNFJavaToNSString(void);
void* JNFNSTimeIntervalToJavaMillis(void);
void* JNFNSToJavaCalendar(void);
void* JNFNSToJavaNumber(void);
void* JNFNSToJavaString(void);
void* JNFNativeMethodEnter(void);
void* JNFNativeMethodExit(void);
void* JNFNewBooleanArray(void);
void* JNFNewByteArray(void);
void* JNFNewCharArray(void);
void* JNFNewDoubleArray(void);
void* JNFNewFloatArray(void);
void* JNFNewGlobalRef(void);
void* JNFNewIntArray(void);
void* JNFNewLongArray(void);
void* JNFNewObject(void);
void* JNFNewObjectArray(void);
void* JNFNewShortArray(void);
void* JNFNewWeakGlobalRef(void);
void* JNFNormalizedJavaStringForPath(void);
void* JNFNormalizedNSStringForPath(void);
void* JNFObjectClassName(void);
void* JNFObjectEquals(void);
void* JNFObjectToString(void);
void* JNFObtainEnv(void);
void* JNFPerformEnvBlock(void);
void* JNFReleaseEnv(void);
void* JNFReleaseStringUTF16UniChars(void);
void* JNFReleaseStringUTF8Chars(void);
void* JNFSetBooleanField(void);
void* JNFSetByteField(void);
void* JNFSetCharField(void);
void* JNFSetDoubleField(void);
void* JNFSetFloatField(void);
void* JNFSetIntField(void);
void* JNFSetLongField(void);
void* JNFSetObjectField(void);
void* JNFSetShortField(void);
void* JNFSetStaticBooleanField(void);
void* JNFSetStaticByteField(void);
void* JNFSetStaticCharField(void);
void* JNFSetStaticDoubleField(void);
void* JNFSetStaticFloatField(void);
void* JNFSetStaticIntField(void);
void* JNFSetStaticLongField(void);
void* JNFSetStaticObjectField(void);
void* JNFSetStaticShortField(void);
void* JNFSetTracePS(void);

#endif
