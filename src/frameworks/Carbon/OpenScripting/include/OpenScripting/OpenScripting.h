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


#ifndef _OpenScripting_H_
#define _OpenScripting_H_

#import <Foundation/Foundation.h>


void* ASCopySourceAttributes(void);
void* ASGetSourceStyleNames(void);
void* ASGetSourceStyles(void);
void* ASInit(void);
void* ASSetSourceAttributes(void);
void* ASSetSourceStyles(void);
void* OSAAddStorageType(void);
void* OSAAuthorizeEvent(void);
void* OSAAvailableDialectCodeList(void);
void* OSAAvailableDialects(void);
void* OSACoerceFromDesc(void);
void* OSACoerceToDesc(void);
void* OSACompile(void);
void* OSACompileExecute(void);
void* OSACopyDisplayString(void);
void* OSACopyID(void);
void* OSACopyScript(void);
void* OSACopyScriptingDefinition(void);
void* OSACopyScriptingDefinitionFromFile(void);
void* OSACopyScriptingDefinitionFromURL(void);
void* OSACopySourceString(void);
void* OSACopyURLForLibraryInDomainURL(void);
void* OSACopyURLForLibraryInFolderURL(void);
void* OSACopyURLForLibraryInScriptURL(void);
void* OSACopyURLForLibraryName(void);
void* OSACreateFile(void);
void* OSADebuggerCreateSession(void);
void* OSADebuggerDisposeCallFrame(void);
void* OSADebuggerDisposeSession(void);
void* OSADebuggerGetBreakpoint(void);
void* OSADebuggerGetCallFrameState(void);
void* OSADebuggerGetCurrentCallFrame(void);
void* OSADebuggerGetDefaultBreakpoint(void);
void* OSADebuggerGetPreviousCallFrame(void);
void* OSADebuggerGetSessionState(void);
void* OSADebuggerGetStatementRanges(void);
void* OSADebuggerGetVariable(void);
void* OSADebuggerSessionStep(void);
void* OSADebuggerSetBreakpoint(void);
void* OSADebuggerSetVariable(void);
void* OSADisplay(void);
void* OSADispose(void);
void* OSADoEvent(void);
void* OSADoScript(void);
void* OSADoScriptFile(void);
void* OSAExecute(void);
void* OSAExecuteEvent(void);
void* OSAFindEventHandler(void);
void* OSAGenericToRealID(void);
void* OSAGetActiveProc(void);
void* OSAGetCreateProc(void);
void* OSAGetCreateProgressProc(void);
void* OSAGetCurrentDialect(void);
void* OSAGetDefaultScriptingComponent(void);
void* OSAGetDialectInfo(void);
void* OSAGetEventHandler(void);
void* OSAGetHandler(void);
void* OSAGetHandlerNames(void);
void* OSAGetProperty(void);
void* OSAGetPropertyNames(void);
void* OSAGetReleaseProgressProc(void);
void* OSAGetResumeDispatchProc(void);
void* OSAGetScriptDataFromURL(void);
void* OSAGetScriptInfo(void);
void* OSAGetScriptingComponent(void);
void* OSAGetScriptingComponentFromStored(void);
void* OSAGetSendProc(void);
void* OSAGetSource(void);
void* OSAGetStorageType(void);
void* OSAGetSysTerminology(void);
void* OSAInstallEventHandler(void);
void* OSAInstallGestaltValues(void);
void* OSAInstallScriptingDescCoercions(void);
void* OSAInstallScriptingEventHandlers(void);
void* OSAInstallStdDescCoercions(void);
void* OSAInstallStdEventHandlers(void);
void* OSAInstallStdPtrCoercions(void);
void* OSAInstallUnitPtrCoercions(void);
void* OSAIsEventAuthorizationRequested(void);
void* OSAIsEventAuthorized(void);
void* OSAIsScriptableBundle(void);
void* OSAIsScriptableRef(void);
void* OSAIsScriptableURL(void);
void* OSALoad(void);
void* OSALoadExecute(void);
void* OSALoadExecuteFile(void);
void* OSALoadFile(void);
void* OSALoadScriptData(void);
void* OSALoadScriptDataFromFile(void);
void* OSAMakeContext(void);
void* OSARealToGenericID(void);
void* OSARecordEvent(void);
void* OSARemoveEventHandler(void);
void* OSARemoveStorageType(void);
void* OSARequestEventAuthorization(void);
void* OSAScriptError(void);
void* OSAScriptingComponentName(void);
void* OSASetActiveProc(void);
void* OSASetCreateProc(void);
void* OSASetCreateProgressProc(void);
void* OSASetCurrentDialect(void);
void* OSASetDefaultScriptingComponent(void);
void* OSASetDefaultTarget(void);
void* OSASetHandler(void);
void* OSASetProperty(void);
void* OSASetReleaseProgressProc(void);
void* OSASetResumeDispatchProc(void);
void* OSASetScriptInfo(void);
void* OSASetSendProc(void);
void* OSAStartRecording(void);
void* OSAStopRecording(void);
void* OSAStore(void);
void* OSAStoreFile(void);
void* OSAStoreScriptDataToFile(void);
void* OSAStoreScriptDataToNewFile(void);
void* _HandleGDUT(void);
void* _OSACopyDisplayString(void);
void* _OSACopyScriptingDefinitionFromFile(void);
void* _OSACopySourceString(void);
void* _OSACreateURL(void);
void* _OSADoScriptURL(void);
void* _OSALoadExecuteURL(void);
void* _OSALoadURL(void);
void* _OSAStoreScriptDataToURL(void);
void* _OSAStoreURL(void);

#endif
