/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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


#include <JavaVM/JavaVM.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CheckForInstalledJavaRuntimes(void) {
    if (verbose) puts("STUB: CheckForInstalledJavaRuntimes called");
    return NULL;
}

void* JAWT_GetAWT(void) {
    if (verbose) puts("STUB: JAWT_GetAWT called");
    return NULL;
}

void* JNI_CreateJavaVM(void) {
    if (verbose) puts("STUB: JNI_CreateJavaVM called");
    return NULL;
}

void* JNI_GetCreatedJavaVMs(void) {
    if (verbose) puts("STUB: JNI_GetCreatedJavaVMs called");
    return NULL;
}

void* JNI_GetDefaultJavaVMInitArgs(void) {
    if (verbose) puts("STUB: JNI_GetDefaultJavaVMInitArgs called");
    return NULL;
}

void* JVMCopyDefaultPrefForTaskAndKey(void) {
    if (verbose) puts("STUB: JVMCopyDefaultPrefForTaskAndKey called");
    return NULL;
}

void* JVMCopyUserPrefForTaskAndKey(void) {
    if (verbose) puts("STUB: JVMCopyUserPrefForTaskAndKey called");
    return NULL;
}

void* JVMCreateCompleteJVMListForTask(void) {
    if (verbose) puts("STUB: JVMCreateCompleteJVMListForTask called");
    return NULL;
}

void* JVMCreateJVMListForTask(void) {
    if (verbose) puts("STUB: JVMCreateJVMListForTask called");
    return NULL;
}

void* JVMCreateJVMListForTaskVersionAndArch(void) {
    if (verbose) puts("STUB: JVMCreateJVMListForTaskVersionAndArch called");
    return NULL;
}

void* JVMCreateJVMListForTaskVersionAndArchs(void) {
    if (verbose) puts("STUB: JVMCreateJVMListForTaskVersionAndArchs called");
    return NULL;
}

void* JVMCreateJVMListOfCurrentArchitectureForTask(void) {
    if (verbose) puts("STUB: JVMCreateJVMListOfCurrentArchitectureForTask called");
    return NULL;
}

void* JVMCreateLatestJVMInfo(void) {
    if (verbose) puts("STUB: JVMCreateLatestJVMInfo called");
    return NULL;
}

void* JVMSaveUserPrefForTaskAndKey(void) {
    if (verbose) puts("STUB: JVMSaveUserPrefForTaskAndKey called");
    return NULL;
}

void* JVMSetPreferredOrderForTask(void) {
    if (verbose) puts("STUB: JVMSetPreferredOrderForTask called");
    return NULL;
}

void* _NSJVMAddressOfFunctionInLibrary(void) {
    if (verbose) puts("STUB: _NSJVMAddressOfFunctionInLibrary called");
    return NULL;
}

void* _NSJVMAddressOfFunctionInLibraryNamed(void) {
    if (verbose) puts("STUB: _NSJVMAddressOfFunctionInLibraryNamed called");
    return NULL;
}

void* _NSJVMInitializeObjCRuntimeInLibraryNamed(void) {
    if (verbose) puts("STUB: _NSJVMInitializeObjCRuntimeInLibraryNamed called");
    return NULL;
}

void* _NSJVMJNI_CreateJavaVM(void) {
    if (verbose) puts("STUB: _NSJVMJNI_CreateJavaVM called");
    return NULL;
}

void* _NSJVMJNI_GetCreatedJavaVMs(void) {
    if (verbose) puts("STUB: _NSJVMJNI_GetCreatedJavaVMs called");
    return NULL;
}

void* _NSJVMJNI_GetDefaultJavaVMInitArgs(void) {
    if (verbose) puts("STUB: _NSJVMJNI_GetDefaultJavaVMInitArgs called");
    return NULL;
}

void* _NSJVMJavaVMLibraryName(void) {
    if (verbose) puts("STUB: _NSJVMJavaVMLibraryName called");
    return NULL;
}

void* _NSJVMLoadLibrary(void) {
    if (verbose) puts("STUB: _NSJVMLoadLibrary called");
    return NULL;
}

void* _NSJVM_IHashCodeFunction(void) {
    if (verbose) puts("STUB: _NSJVM_IHashCodeFunction called");
    return NULL;
}

void* _NSJVM_IsJVMLoaded(void) {
    if (verbose) puts("STUB: _NSJVM_IsJVMLoaded called");
    return NULL;
}

void* _NSLoadJavaVirtualMachine(void) {
    if (verbose) puts("STUB: _NSLoadJavaVirtualMachine called");
    return NULL;
}

void* scheduleUpdateSharing(void) {
    if (verbose) puts("STUB: scheduleUpdateSharing called");
    return NULL;
}

void* scheduleUpdateSharingUninstalled(void) {
    if (verbose) puts("STUB: scheduleUpdateSharingUninstalled called");
    return NULL;
}

void* updateSharing(void) {
    if (verbose) puts("STUB: updateSharing called");
    return NULL;
}
