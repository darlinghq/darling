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


#include <OpenScripting/OpenScripting.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* ASCopySourceAttributes(void)
{
    if (verbose) puts("STUB: ASCopySourceAttributes called");
    return NULL;
}

void* ASGetSourceStyleNames(void)
{
    if (verbose) puts("STUB: ASGetSourceStyleNames called");
    return NULL;
}

void* ASGetSourceStyles(void)
{
    if (verbose) puts("STUB: ASGetSourceStyles called");
    return NULL;
}

void* ASInit(void)
{
    if (verbose) puts("STUB: ASInit called");
    return NULL;
}

void* ASSetSourceAttributes(void)
{
    if (verbose) puts("STUB: ASSetSourceAttributes called");
    return NULL;
}

void* ASSetSourceStyles(void)
{
    if (verbose) puts("STUB: ASSetSourceStyles called");
    return NULL;
}

void* OSAAddStorageType(void)
{
    if (verbose) puts("STUB: OSAAddStorageType called");
    return NULL;
}

void* OSAAuthorizeEvent(void)
{
    if (verbose) puts("STUB: OSAAuthorizeEvent called");
    return NULL;
}

void* OSAAvailableDialectCodeList(void)
{
    if (verbose) puts("STUB: OSAAvailableDialectCodeList called");
    return NULL;
}

void* OSAAvailableDialects(void)
{
    if (verbose) puts("STUB: OSAAvailableDialects called");
    return NULL;
}

void* OSACoerceFromDesc(void)
{
    if (verbose) puts("STUB: OSACoerceFromDesc called");
    return NULL;
}

void* OSACoerceToDesc(void)
{
    if (verbose) puts("STUB: OSACoerceToDesc called");
    return NULL;
}

void* OSACompile(void)
{
    if (verbose) puts("STUB: OSACompile called");
    return NULL;
}

void* OSACompileExecute(void)
{
    if (verbose) puts("STUB: OSACompileExecute called");
    return NULL;
}

void* OSACopyDisplayString(void)
{
    if (verbose) puts("STUB: OSACopyDisplayString called");
    return NULL;
}

void* OSACopyID(void)
{
    if (verbose) puts("STUB: OSACopyID called");
    return NULL;
}

void* OSACopyScript(void)
{
    if (verbose) puts("STUB: OSACopyScript called");
    return NULL;
}

void* OSACopyScriptingDefinition(void)
{
    if (verbose) puts("STUB: OSACopyScriptingDefinition called");
    return NULL;
}

void* OSACopyScriptingDefinitionFromFile(void)
{
    if (verbose) puts("STUB: OSACopyScriptingDefinitionFromFile called");
    return NULL;
}

void* OSACopyScriptingDefinitionFromURL(void)
{
    if (verbose) puts("STUB: OSACopyScriptingDefinitionFromURL called");
    return NULL;
}

void* OSACopySourceString(void)
{
    if (verbose) puts("STUB: OSACopySourceString called");
    return NULL;
}

void* OSACopyURLForLibraryInDomainURL(void)
{
    if (verbose) puts("STUB: OSACopyURLForLibraryInDomainURL called");
    return NULL;
}

void* OSACopyURLForLibraryInFolderURL(void)
{
    if (verbose) puts("STUB: OSACopyURLForLibraryInFolderURL called");
    return NULL;
}

void* OSACopyURLForLibraryInScriptURL(void)
{
    if (verbose) puts("STUB: OSACopyURLForLibraryInScriptURL called");
    return NULL;
}

void* OSACopyURLForLibraryName(void)
{
    if (verbose) puts("STUB: OSACopyURLForLibraryName called");
    return NULL;
}

void* OSACreateFile(void)
{
    if (verbose) puts("STUB: OSACreateFile called");
    return NULL;
}

void* OSADebuggerCreateSession(void)
{
    if (verbose) puts("STUB: OSADebuggerCreateSession called");
    return NULL;
}

void* OSADebuggerDisposeCallFrame(void)
{
    if (verbose) puts("STUB: OSADebuggerDisposeCallFrame called");
    return NULL;
}

void* OSADebuggerDisposeSession(void)
{
    if (verbose) puts("STUB: OSADebuggerDisposeSession called");
    return NULL;
}

void* OSADebuggerGetBreakpoint(void)
{
    if (verbose) puts("STUB: OSADebuggerGetBreakpoint called");
    return NULL;
}

void* OSADebuggerGetCallFrameState(void)
{
    if (verbose) puts("STUB: OSADebuggerGetCallFrameState called");
    return NULL;
}

void* OSADebuggerGetCurrentCallFrame(void)
{
    if (verbose) puts("STUB: OSADebuggerGetCurrentCallFrame called");
    return NULL;
}

void* OSADebuggerGetDefaultBreakpoint(void)
{
    if (verbose) puts("STUB: OSADebuggerGetDefaultBreakpoint called");
    return NULL;
}

void* OSADebuggerGetPreviousCallFrame(void)
{
    if (verbose) puts("STUB: OSADebuggerGetPreviousCallFrame called");
    return NULL;
}

void* OSADebuggerGetSessionState(void)
{
    if (verbose) puts("STUB: OSADebuggerGetSessionState called");
    return NULL;
}

void* OSADebuggerGetStatementRanges(void)
{
    if (verbose) puts("STUB: OSADebuggerGetStatementRanges called");
    return NULL;
}

void* OSADebuggerGetVariable(void)
{
    if (verbose) puts("STUB: OSADebuggerGetVariable called");
    return NULL;
}

void* OSADebuggerSessionStep(void)
{
    if (verbose) puts("STUB: OSADebuggerSessionStep called");
    return NULL;
}

void* OSADebuggerSetBreakpoint(void)
{
    if (verbose) puts("STUB: OSADebuggerSetBreakpoint called");
    return NULL;
}

void* OSADebuggerSetVariable(void)
{
    if (verbose) puts("STUB: OSADebuggerSetVariable called");
    return NULL;
}

void* OSADisplay(void)
{
    if (verbose) puts("STUB: OSADisplay called");
    return NULL;
}

void* OSADispose(void)
{
    if (verbose) puts("STUB: OSADispose called");
    return NULL;
}

void* OSADoEvent(void)
{
    if (verbose) puts("STUB: OSADoEvent called");
    return NULL;
}

void* OSADoScript(void)
{
    if (verbose) puts("STUB: OSADoScript called");
    return NULL;
}

void* OSADoScriptFile(void)
{
    if (verbose) puts("STUB: OSADoScriptFile called");
    return NULL;
}

void* OSAExecute(void)
{
    if (verbose) puts("STUB: OSAExecute called");
    return NULL;
}

void* OSAExecuteEvent(void)
{
    if (verbose) puts("STUB: OSAExecuteEvent called");
    return NULL;
}

void* OSAFindEventHandler(void)
{
    if (verbose) puts("STUB: OSAFindEventHandler called");
    return NULL;
}

void* OSAGenericToRealID(void)
{
    if (verbose) puts("STUB: OSAGenericToRealID called");
    return NULL;
}

void* OSAGetActiveProc(void)
{
    if (verbose) puts("STUB: OSAGetActiveProc called");
    return NULL;
}

void* OSAGetCreateProc(void)
{
    if (verbose) puts("STUB: OSAGetCreateProc called");
    return NULL;
}

void* OSAGetCreateProgressProc(void)
{
    if (verbose) puts("STUB: OSAGetCreateProgressProc called");
    return NULL;
}

void* OSAGetCurrentDialect(void)
{
    if (verbose) puts("STUB: OSAGetCurrentDialect called");
    return NULL;
}

void* OSAGetDefaultScriptingComponent(void)
{
    if (verbose) puts("STUB: OSAGetDefaultScriptingComponent called");
    return NULL;
}

void* OSAGetDialectInfo(void)
{
    if (verbose) puts("STUB: OSAGetDialectInfo called");
    return NULL;
}

void* OSAGetEventHandler(void)
{
    if (verbose) puts("STUB: OSAGetEventHandler called");
    return NULL;
}

void* OSAGetHandler(void)
{
    if (verbose) puts("STUB: OSAGetHandler called");
    return NULL;
}

void* OSAGetHandlerNames(void)
{
    if (verbose) puts("STUB: OSAGetHandlerNames called");
    return NULL;
}

void* OSAGetProperty(void)
{
    if (verbose) puts("STUB: OSAGetProperty called");
    return NULL;
}

void* OSAGetPropertyNames(void)
{
    if (verbose) puts("STUB: OSAGetPropertyNames called");
    return NULL;
}

void* OSAGetReleaseProgressProc(void)
{
    if (verbose) puts("STUB: OSAGetReleaseProgressProc called");
    return NULL;
}

void* OSAGetResumeDispatchProc(void)
{
    if (verbose) puts("STUB: OSAGetResumeDispatchProc called");
    return NULL;
}

void* OSAGetScriptDataFromURL(void)
{
    if (verbose) puts("STUB: OSAGetScriptDataFromURL called");
    return NULL;
}

void* OSAGetScriptInfo(void)
{
    if (verbose) puts("STUB: OSAGetScriptInfo called");
    return NULL;
}

void* OSAGetScriptingComponent(void)
{
    if (verbose) puts("STUB: OSAGetScriptingComponent called");
    return NULL;
}

void* OSAGetScriptingComponentFromStored(void)
{
    if (verbose) puts("STUB: OSAGetScriptingComponentFromStored called");
    return NULL;
}

void* OSAGetSendProc(void)
{
    if (verbose) puts("STUB: OSAGetSendProc called");
    return NULL;
}

void* OSAGetSource(void)
{
    if (verbose) puts("STUB: OSAGetSource called");
    return NULL;
}

void* OSAGetStorageType(void)
{
    if (verbose) puts("STUB: OSAGetStorageType called");
    return NULL;
}

void* OSAGetSysTerminology(void)
{
    if (verbose) puts("STUB: OSAGetSysTerminology called");
    return NULL;
}

void* OSAInstallEventHandler(void)
{
    if (verbose) puts("STUB: OSAInstallEventHandler called");
    return NULL;
}

void* OSAInstallGestaltValues(void)
{
    if (verbose) puts("STUB: OSAInstallGestaltValues called");
    return NULL;
}

void* OSAInstallScriptingDescCoercions(void)
{
    if (verbose) puts("STUB: OSAInstallScriptingDescCoercions called");
    return NULL;
}

void* OSAInstallScriptingEventHandlers(void)
{
    if (verbose) puts("STUB: OSAInstallScriptingEventHandlers called");
    return NULL;
}

void* OSAInstallStdDescCoercions(void)
{
    if (verbose) puts("STUB: OSAInstallStdDescCoercions called");
    return NULL;
}

void* OSAInstallStdEventHandlers(void)
{
    if (verbose) puts("STUB: OSAInstallStdEventHandlers called");
    return NULL;
}

void* OSAInstallStdPtrCoercions(void)
{
    if (verbose) puts("STUB: OSAInstallStdPtrCoercions called");
    return NULL;
}

void* OSAInstallUnitPtrCoercions(void)
{
    if (verbose) puts("STUB: OSAInstallUnitPtrCoercions called");
    return NULL;
}

void* OSAIsEventAuthorizationRequested(void)
{
    if (verbose) puts("STUB: OSAIsEventAuthorizationRequested called");
    return NULL;
}

void* OSAIsEventAuthorized(void)
{
    if (verbose) puts("STUB: OSAIsEventAuthorized called");
    return NULL;
}

void* OSAIsScriptableBundle(void)
{
    if (verbose) puts("STUB: OSAIsScriptableBundle called");
    return NULL;
}

void* OSAIsScriptableRef(void)
{
    if (verbose) puts("STUB: OSAIsScriptableRef called");
    return NULL;
}

void* OSAIsScriptableURL(void)
{
    if (verbose) puts("STUB: OSAIsScriptableURL called");
    return NULL;
}

void* OSALoad(void)
{
    if (verbose) puts("STUB: OSALoad called");
    return NULL;
}

void* OSALoadExecute(void)
{
    if (verbose) puts("STUB: OSALoadExecute called");
    return NULL;
}

void* OSALoadExecuteFile(void)
{
    if (verbose) puts("STUB: OSALoadExecuteFile called");
    return NULL;
}

void* OSALoadFile(void)
{
    if (verbose) puts("STUB: OSALoadFile called");
    return NULL;
}

void* OSALoadScriptData(void)
{
    if (verbose) puts("STUB: OSALoadScriptData called");
    return NULL;
}

void* OSALoadScriptDataFromFile(void)
{
    if (verbose) puts("STUB: OSALoadScriptDataFromFile called");
    return NULL;
}

void* OSAMakeContext(void)
{
    if (verbose) puts("STUB: OSAMakeContext called");
    return NULL;
}

void* OSARealToGenericID(void)
{
    if (verbose) puts("STUB: OSARealToGenericID called");
    return NULL;
}

void* OSARecordEvent(void)
{
    if (verbose) puts("STUB: OSARecordEvent called");
    return NULL;
}

void* OSARemoveEventHandler(void)
{
    if (verbose) puts("STUB: OSARemoveEventHandler called");
    return NULL;
}

void* OSARemoveStorageType(void)
{
    if (verbose) puts("STUB: OSARemoveStorageType called");
    return NULL;
}

void* OSARequestEventAuthorization(void)
{
    if (verbose) puts("STUB: OSARequestEventAuthorization called");
    return NULL;
}

void* OSAScriptError(void)
{
    if (verbose) puts("STUB: OSAScriptError called");
    return NULL;
}

void* OSAScriptingComponentName(void)
{
    if (verbose) puts("STUB: OSAScriptingComponentName called");
    return NULL;
}

void* OSASetActiveProc(void)
{
    if (verbose) puts("STUB: OSASetActiveProc called");
    return NULL;
}

void* OSASetCreateProc(void)
{
    if (verbose) puts("STUB: OSASetCreateProc called");
    return NULL;
}

void* OSASetCreateProgressProc(void)
{
    if (verbose) puts("STUB: OSASetCreateProgressProc called");
    return NULL;
}

void* OSASetCurrentDialect(void)
{
    if (verbose) puts("STUB: OSASetCurrentDialect called");
    return NULL;
}

void* OSASetDefaultScriptingComponent(void)
{
    if (verbose) puts("STUB: OSASetDefaultScriptingComponent called");
    return NULL;
}

void* OSASetDefaultTarget(void)
{
    if (verbose) puts("STUB: OSASetDefaultTarget called");
    return NULL;
}

void* OSASetHandler(void)
{
    if (verbose) puts("STUB: OSASetHandler called");
    return NULL;
}

void* OSASetProperty(void)
{
    if (verbose) puts("STUB: OSASetProperty called");
    return NULL;
}

void* OSASetReleaseProgressProc(void)
{
    if (verbose) puts("STUB: OSASetReleaseProgressProc called");
    return NULL;
}

void* OSASetResumeDispatchProc(void)
{
    if (verbose) puts("STUB: OSASetResumeDispatchProc called");
    return NULL;
}

void* OSASetScriptInfo(void)
{
    if (verbose) puts("STUB: OSASetScriptInfo called");
    return NULL;
}

void* OSASetSendProc(void)
{
    if (verbose) puts("STUB: OSASetSendProc called");
    return NULL;
}

void* OSAStartRecording(void)
{
    if (verbose) puts("STUB: OSAStartRecording called");
    return NULL;
}

void* OSAStopRecording(void)
{
    if (verbose) puts("STUB: OSAStopRecording called");
    return NULL;
}

void* OSAStore(void)
{
    if (verbose) puts("STUB: OSAStore called");
    return NULL;
}

void* OSAStoreFile(void)
{
    if (verbose) puts("STUB: OSAStoreFile called");
    return NULL;
}

void* OSAStoreScriptDataToFile(void)
{
    if (verbose) puts("STUB: OSAStoreScriptDataToFile called");
    return NULL;
}

void* OSAStoreScriptDataToNewFile(void)
{
    if (verbose) puts("STUB: OSAStoreScriptDataToNewFile called");
    return NULL;
}

void* _HandleGDUT(void)
{
    if (verbose) puts("STUB: _HandleGDUT called");
    return NULL;
}

void* _OSACopyDisplayString(void)
{
    if (verbose) puts("STUB: _OSACopyDisplayString called");
    return NULL;
}

void* _OSACopyScriptingDefinitionFromFile(void)
{
    if (verbose) puts("STUB: _OSACopyScriptingDefinitionFromFile called");
    return NULL;
}

void* _OSACopySourceString(void)
{
    if (verbose) puts("STUB: _OSACopySourceString called");
    return NULL;
}

void* _OSACreateURL(void)
{
    if (verbose) puts("STUB: _OSACreateURL called");
    return NULL;
}

void* _OSADoScriptURL(void)
{
    if (verbose) puts("STUB: _OSADoScriptURL called");
    return NULL;
}

void* _OSALoadExecuteURL(void)
{
    if (verbose) puts("STUB: _OSALoadExecuteURL called");
    return NULL;
}

void* _OSALoadURL(void)
{
    if (verbose) puts("STUB: _OSALoadURL called");
    return NULL;
}

void* _OSAStoreScriptDataToURL(void)
{
    if (verbose) puts("STUB: _OSAStoreScriptDataToURL called");
    return NULL;
}

void* _OSAStoreURL(void)
{
    if (verbose) puts("STUB: _OSAStoreURL called");
    return NULL;
}
