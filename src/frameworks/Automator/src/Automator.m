/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#include <Automator/Automator.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AMApplicationStubMain(void)
{
    if (verbose) puts("STUB: AMApplicationStubMain called");
    return NULL;
}

void* AMErrorWithInfo(void)
{
    if (verbose) puts("STUB: AMErrorWithInfo called");
    return NULL;
}

void* AMErrorWithMessage(void)
{
    if (verbose) puts("STUB: AMErrorWithMessage called");
    return NULL;
}

void* AMLocalizedStringWithTemplate(void)
{
    if (verbose) puts("STUB: AMLocalizedStringWithTemplate called");
    return NULL;
}

void* AMPathFromAliasDescriptor(void)
{
    if (verbose) puts("STUB: AMPathFromAliasDescriptor called");
    return NULL;
}

void* AMPlainTextFromAttributedString(void)
{
    if (verbose) puts("STUB: AMPlainTextFromAttributedString called");
    return NULL;
}

void* AMRectFromPoints(void)
{
    if (verbose) puts("STUB: AMRectFromPoints called");
    return NULL;
}

void* AMRoundedRectPath(void)
{
    if (verbose) puts("STUB: AMRoundedRectPath called");
    return NULL;
}

void* AMRunActionPanel(void)
{
    if (verbose) puts("STUB: AMRunActionPanel called");
    return NULL;
}

void* AMSBObjectFromObjectDescriptor(void)
{
    if (verbose) puts("STUB: AMSBObjectFromObjectDescriptor called");
    return NULL;
}

void* AMSecAssessIsURLSignedByApple(void)
{
    if (verbose) puts("STUB: AMSecAssessIsURLSignedByApple called");
    return NULL;
}

void* AMStringObjectWithArray(void)
{
    if (verbose) puts("STUB: AMStringObjectWithArray called");
    return NULL;
}

void* AM_ActiveProc(void)
{
    if (verbose) puts("STUB: AM_ActiveProc called");
    return NULL;
}

void* AM_FilterProc(void)
{
    if (verbose) puts("STUB: AM_FilterProc called");
    return NULL;
}

void* AM_IdleProc(void)
{
    if (verbose) puts("STUB: AM_IdleProc called");
    return NULL;
}

void* AM_SendProc(void)
{
    if (verbose) puts("STUB: AM_SendProc called");
    return NULL;
}

void* AutomatorSecurityHelperHostInterface(void)
{
    if (verbose) puts("STUB: AutomatorSecurityHelperHostInterface called");
    return NULL;
}

void* AutomatorSecurityHelperServiceInterface(void)
{
    if (verbose) puts("STUB: AutomatorSecurityHelperServiceInterface called");
    return NULL;
}

void* _AMAllowDeprecatedActions(void)
{
    if (verbose) puts("STUB: _AMAllowDeprecatedActions called");
    return NULL;
}

void* _AMAuditTokenForCurrentProcess(void)
{
    if (verbose) puts("STUB: _AMAuditTokenForCurrentProcess called");
    return NULL;
}

void* _AMAutomatorApplicationStubBundle(void)
{
    if (verbose) puts("STUB: _AMAutomatorApplicationStubBundle called");
    return NULL;
}

void* _AMAutomatorFrameworkBundle(void)
{
    if (verbose) puts("STUB: _AMAutomatorFrameworkBundle called");
    return NULL;
}

void* _AMAutomatorFrameworkColor(void)
{
    if (verbose) puts("STUB: _AMAutomatorFrameworkColor called");
    return NULL;
}

void* _AMAutomatorFrameworkString(void)
{
    if (verbose) puts("STUB: _AMAutomatorFrameworkString called");
    return NULL;
}

void* _AMBoolForSetting(void)
{
    if (verbose) puts("STUB: _AMBoolForSetting called");
    return NULL;
}

void* _AMBundleIsLinkedOnMacOS10_14_Or_Later(void)
{
    if (verbose) puts("STUB: _AMBundleIsLinkedOnMacOS10_14_Or_Later called");
    return NULL;
}

void* _AMDateByAddingDays(void)
{
    if (verbose) puts("STUB: _AMDateByAddingDays called");
    return NULL;
}

void* _AMDateByAddingYears(void)
{
    if (verbose) puts("STUB: _AMDateByAddingYears called");
    return NULL;
}

void* _AMDateThisYear(void)
{
    if (verbose) puts("STUB: _AMDateThisYear called");
    return NULL;
}

void* _AMDebugDirectoryPath(void)
{
    if (verbose) puts("STUB: _AMDebugDirectoryPath called");
    return NULL;
}

void* _AMDisableTemplateChooser(void)
{
    if (verbose) puts("STUB: _AMDisableTemplateChooser called");
    return NULL;
}

void* _AMDisplayProgressInMenuBar(void)
{
    if (verbose) puts("STUB: _AMDisplayProgressInMenuBar called");
    return NULL;
}

void* _AMIgnoreApplicationActions(void)
{
    if (verbose) puts("STUB: _AMIgnoreApplicationActions called");
    return NULL;
}

void* _AMIgnoreApplicationValidation(void)
{
    if (verbose) puts("STUB: _AMIgnoreApplicationValidation called");
    return NULL;
}

void* _AMIgnoreSystemActions(void)
{
    if (verbose) puts("STUB: _AMIgnoreSystemActions called");
    return NULL;
}

void* _AMIsAppleInternal(void)
{
    if (verbose) puts("STUB: _AMIsAppleInternal called");
    return NULL;
}

void* _AMLauncherDebugPath(void)
{
    if (verbose) puts("STUB: _AMLauncherDebugPath called");
    return NULL;
}

void* _AMLoadOnlyActionsFromOtherTeams(void)
{
    if (verbose) puts("STUB: _AMLoadOnlyActionsFromOtherTeams called");
    return NULL;
}

void* _AMLogConversionPaths(void)
{
    if (verbose) puts("STUB: _AMLogConversionPaths called");
    return NULL;
}

void* _AMMidnightFollowingDate(void)
{
    if (verbose) puts("STUB: _AMMidnightFollowingDate called");
    return NULL;
}

void* _AMMidnightOfDate(void)
{
    if (verbose) puts("STUB: _AMMidnightOfDate called");
    return NULL;
}

void* _AMRunAppleScriptsOnMainThreadOnly(void)
{
    if (verbose) puts("STUB: _AMRunAppleScriptsOnMainThreadOnly called");
    return NULL;
}

void* _AMRunLocally(void)
{
    if (verbose) puts("STUB: _AMRunLocally called");
    return NULL;
}

void* _AMRunningInAutomatorApp(void)
{
    if (verbose) puts("STUB: _AMRunningInAutomatorApp called");
    return NULL;
}

void* _AMStopBeforeStartingPoints(void)
{
    if (verbose) puts("STUB: _AMStopBeforeStartingPoints called");
    return NULL;
}

void* _AMThirdPartyActionsAllowed(void)
{
    if (verbose) puts("STUB: _AMThirdPartyActionsAllowed called");
    return NULL;
}

void* _AMTransferEnvironmentVariablesToXPCService(void)
{
    if (verbose) puts("STUB: _AMTransferEnvironmentVariablesToXPCService called");
    return NULL;
}

void* _AMUIntegerForSetting(void)
{
    if (verbose) puts("STUB: _AMUIntegerForSetting called");
    return NULL;
}

void* _AMURLToAutomatorApp(void)
{
    if (verbose) puts("STUB: _AMURLToAutomatorApp called");
    return NULL;
}

void* _am_EKCalendarItem_convertForSendingOverXPCWithWorkflow(void)
{
    if (verbose) puts("STUB: _am_EKCalendarItem_convertForSendingOverXPCWithWorkflow called");
    return NULL;
}

void* _am_EKCalendar_convertForSendingOverXPCWithWorkflow(void)
{
    if (verbose) puts("STUB: _am_EKCalendar_convertForSendingOverXPCWithWorkflow called");
    return NULL;
}

void* _am_WebArchive_convertForSendingOverXPCWithWorkflow(void)
{
    if (verbose) puts("STUB: _am_WebArchive_convertForSendingOverXPCWithWorkflow called");
    return NULL;
}

void* am_CGFloatAlmostEqual(void)
{
    if (verbose) puts("STUB: am_CGFloatAlmostEqual called");
    return NULL;
}

void* am_DoubleAlmostEqual(void)
{
    if (verbose) puts("STUB: am_DoubleAlmostEqual called");
    return NULL;
}

void* am_FloatAlmostEqual(void)
{
    if (verbose) puts("STUB: am_FloatAlmostEqual called");
    return NULL;
}

void* mt_log_automation_app_launched(void)
{
    if (verbose) puts("STUB: mt_log_automation_app_launched called");
    return NULL;
}

void* mt_log_automation_applet_launched(void)
{
    if (verbose) puts("STUB: mt_log_automation_applet_launched called");
    return NULL;
}

void* workflow_service_runner_peer_handler(void)
{
    if (verbose) puts("STUB: workflow_service_runner_peer_handler called");
    return NULL;
}

void* workflow_service_runner_xpc_main(void)
{
    if (verbose) puts("STUB: workflow_service_runner_xpc_main called");
    return NULL;
}
