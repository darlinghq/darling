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


#ifndef _JavaVM_H_
#define _JavaVM_H_

#import <Foundation/Foundation.h>

#import <JavaVM/NSJavaVirtualMachine.h>
#import <JavaVM/NSJavaConfiguration.h>

void* CheckForInstalledJavaRuntimes(void);
void* JAWT_GetAWT(void);
void* JNI_CreateJavaVM(void);
void* JNI_GetCreatedJavaVMs(void);
void* JNI_GetDefaultJavaVMInitArgs(void);
void* JVMCopyDefaultPrefForTaskAndKey(void);
void* JVMCopyUserPrefForTaskAndKey(void);
void* JVMCreateCompleteJVMListForTask(void);
void* JVMCreateJVMListForTask(void);
void* JVMCreateJVMListForTaskVersionAndArch(void);
void* JVMCreateJVMListForTaskVersionAndArchs(void);
void* JVMCreateJVMListOfCurrentArchitectureForTask(void);
void* JVMCreateLatestJVMInfo(void);
void* JVMSaveUserPrefForTaskAndKey(void);
void* JVMSetPreferredOrderForTask(void);
void* _NSJVMAddressOfFunctionInLibrary(void);
void* _NSJVMAddressOfFunctionInLibraryNamed(void);
void* _NSJVMInitializeObjCRuntimeInLibraryNamed(void);
void* _NSJVMJNI_CreateJavaVM(void);
void* _NSJVMJNI_GetCreatedJavaVMs(void);
void* _NSJVMJNI_GetDefaultJavaVMInitArgs(void);
void* _NSJVMJavaVMLibraryName(void);
void* _NSJVMLoadLibrary(void);
void* _NSJVM_IHashCodeFunction(void);
void* _NSJVM_IsJVMLoaded(void);
void* _NSLoadJavaVirtualMachine(void);
void* scheduleUpdateSharing(void);
void* scheduleUpdateSharingUninstalled(void);
void* updateSharing(void);

#endif
