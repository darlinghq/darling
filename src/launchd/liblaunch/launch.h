/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __LAUNCH_H__
#define __LAUNCH_H__

#include <mach/mach.h>
#include <sys/cdefs.h>
#include <stddef.h>
#include <stdbool.h>

#pragma GCC visibility push(default)

__BEGIN_DECLS

#ifdef __GNUC__
#define __ld_normal __attribute__((__nothrow__))
#define __ld_setter __attribute__((__nothrow__, __nonnull__))
#define __ld_getter __attribute__((__nothrow__, __nonnull__, __pure__, __warn_unused_result__))
#define __ld_iterator(x, y) __attribute__((__nonnull__(x, y)))
#define __ld_allocator __attribute__((__nothrow__, __malloc__, __nonnull__, __warn_unused_result__))
#else
#define __ld_normal
#define __ld_setter
#define __ld_getter
#define __ld_iterator(x, y)
#define __ld_allocator
#endif

#define LAUNCH_KEY_SUBMITJOB "SubmitJob"
#define LAUNCH_KEY_REMOVEJOB "RemoveJob"
#define LAUNCH_KEY_STARTJOB "StartJob"
#define LAUNCH_KEY_STOPJOB "StopJob"
#define LAUNCH_KEY_GETJOB "GetJob"
#define LAUNCH_KEY_GETJOBS "GetJobs"
#define LAUNCH_KEY_CHECKIN "CheckIn"

#define LAUNCH_JOBKEY_DEFAULTS "__Defaults"

#define LAUNCH_JOBKEY_LABEL "Label"
#define LAUNCH_JOBKEY_DISABLED "Disabled"
#define LAUNCH_JOBKEY_USERNAME "UserName"
#define LAUNCH_JOBKEY_GROUPNAME "GroupName"
#define LAUNCH_JOBKEY_TIMEOUT "TimeOut"
#define LAUNCH_JOBKEY_EXITTIMEOUT "ExitTimeOut"
#define LAUNCH_JOBKEY_INITGROUPS "InitGroups"
#define LAUNCH_JOBKEY_SOCKETS "Sockets"
#define LAUNCH_JOBKEY_MACHSERVICES "MachServices"
#define LAUNCH_JOBKEY_MACHSERVICELOOKUPPOLICIES "MachServiceLookupPolicies"
#define LAUNCH_JOBKEY_INETDCOMPATIBILITY "inetdCompatibility"
#define LAUNCH_JOBKEY_ENABLEGLOBBING "EnableGlobbing"
#define LAUNCH_JOBKEY_PROGRAMARGUMENTS "ProgramArguments"
#define LAUNCH_JOBKEY_PROGRAM "Program"
#define LAUNCH_JOBKEY_ONDEMAND "OnDemand"
#define LAUNCH_JOBKEY_KEEPALIVE "KeepAlive"
#define LAUNCH_JOBKEY_LIMITLOADTOHOSTS "LimitLoadToHosts"
#define LAUNCH_JOBKEY_LIMITLOADFROMHOSTS "LimitLoadFromHosts"
#define LAUNCH_JOBKEY_LIMITLOADTOSESSIONTYPE "LimitLoadToSessionType"
#define LAUNCH_JOBKEY_LIMITLOADTOHARDWARE "LimitLoadToHardware"
#define LAUNCH_JOBKEY_LIMITLOADFROMHARDWARE "LimitLoadFromHardware"
#define LAUNCH_JOBKEY_RUNATLOAD "RunAtLoad"
#define LAUNCH_JOBKEY_ROOTDIRECTORY "RootDirectory"
#define LAUNCH_JOBKEY_WORKINGDIRECTORY "WorkingDirectory"
#define LAUNCH_JOBKEY_ENVIRONMENTVARIABLES "EnvironmentVariables"
#define LAUNCH_JOBKEY_USERENVIRONMENTVARIABLES "UserEnvironmentVariables"
#define LAUNCH_JOBKEY_UMASK "Umask"
#define LAUNCH_JOBKEY_NICE "Nice"
#define LAUNCH_JOBKEY_HOPEFULLYEXITSFIRST "HopefullyExitsFirst"
#define LAUNCH_JOBKEY_HOPEFULLYEXITSLAST "HopefullyExitsLast"
#define LAUNCH_JOBKEY_LOWPRIORITYIO "LowPriorityIO"
#define LAUNCH_JOBKEY_SESSIONCREATE "SessionCreate"
#define LAUNCH_JOBKEY_STARTONMOUNT "StartOnMount"
#define LAUNCH_JOBKEY_SOFTRESOURCELIMITS "SoftResourceLimits"
#define LAUNCH_JOBKEY_HARDRESOURCELIMITS "HardResourceLimits"
#define LAUNCH_JOBKEY_STANDARDINPATH "StandardInPath"
#define LAUNCH_JOBKEY_STANDARDOUTPATH "StandardOutPath"
#define LAUNCH_JOBKEY_STANDARDERRORPATH "StandardErrorPath"
#define LAUNCH_JOBKEY_DEBUG "Debug"
#define LAUNCH_JOBKEY_WAITFORDEBUGGER "WaitForDebugger"
#define LAUNCH_JOBKEY_QUEUEDIRECTORIES "QueueDirectories"
#define LAUNCH_JOBKEY_WATCHPATHS "WatchPaths"
#define LAUNCH_JOBKEY_STARTINTERVAL "StartInterval"
#define LAUNCH_JOBKEY_STARTCALENDARINTERVAL "StartCalendarInterval"
#define LAUNCH_JOBKEY_BONJOURFDS "BonjourFDs"
#define LAUNCH_JOBKEY_LASTEXITSTATUS "LastExitStatus"
#define LAUNCH_JOBKEY_PID "PID"
#define LAUNCH_JOBKEY_THROTTLEINTERVAL "ThrottleInterval"
#define LAUNCH_JOBKEY_LAUNCHONLYONCE "LaunchOnlyOnce"
#define LAUNCH_JOBKEY_ABANDONPROCESSGROUP "AbandonProcessGroup"
#define LAUNCH_JOBKEY_IGNOREPROCESSGROUPATSHUTDOWN	"IgnoreProcessGroupAtShutdown"
#define LAUNCH_JOBKEY_POLICIES "Policies"
#define LAUNCH_JOBKEY_ENABLETRANSACTIONS "EnableTransactions"
#define LAUNCH_JOBKEY_CFBUNDLEIDENTIFIER "CFBundleIdentifier"
#define LAUNCH_JOBKEY_PROCESSTYPE "ProcessType"
#define LAUNCH_KEY_PROCESSTYPE_APP "App"
#define LAUNCH_KEY_PROCESSTYPE_STANDARD "Standard"
#define LAUNCH_KEY_PROCESSTYPE_BACKGROUND "Background"
#define LAUNCH_KEY_PROCESSTYPE_INTERACTIVE "Interactive"
#define LAUNCH_KEY_PROCESSTYPE_ADAPTIVE "Adaptive"

#define LAUNCH_JOBPOLICY_DENYCREATINGOTHERJOBS "DenyCreatingOtherJobs"

#define LAUNCH_JOBINETDCOMPATIBILITY_WAIT "Wait"

#define LAUNCH_JOBKEY_MACH_RESETATCLOSE "ResetAtClose"
#define LAUNCH_JOBKEY_MACH_HIDEUNTILCHECKIN "HideUntilCheckIn"
#define LAUNCH_JOBKEY_MACH_DRAINMESSAGESONCRASH "DrainMessagesOnCrash"
#define LAUNCH_JOBKEY_MACH_PINGEVENTUPDATES "PingEventUpdates"

#define LAUNCH_JOBKEY_KEEPALIVE_SUCCESSFULEXIT "SuccessfulExit"
#define LAUNCH_JOBKEY_KEEPALIVE_NETWORKSTATE "NetworkState"
#define LAUNCH_JOBKEY_KEEPALIVE_PATHSTATE "PathState"
#define LAUNCH_JOBKEY_KEEPALIVE_OTHERJOBACTIVE "OtherJobActive"
#define LAUNCH_JOBKEY_KEEPALIVE_OTHERJOBENABLED "OtherJobEnabled"
#define LAUNCH_JOBKEY_KEEPALIVE_AFTERINITIALDEMAND	"AfterInitialDemand"
#define LAUNCH_JOBKEY_KEEPALIVE_CRASHED "Crashed"

#define LAUNCH_JOBKEY_LAUNCHEVENTS "LaunchEvents"

#define LAUNCH_JOBKEY_CAL_MINUTE "Minute"
#define LAUNCH_JOBKEY_CAL_HOUR "Hour"
#define LAUNCH_JOBKEY_CAL_DAY "Day"
#define LAUNCH_JOBKEY_CAL_WEEKDAY "Weekday"
#define LAUNCH_JOBKEY_CAL_MONTH "Month"

#define LAUNCH_JOBKEY_RESOURCELIMIT_CORE "Core"
#define LAUNCH_JOBKEY_RESOURCELIMIT_CPU "CPU"
#define LAUNCH_JOBKEY_RESOURCELIMIT_DATA "Data"
#define LAUNCH_JOBKEY_RESOURCELIMIT_FSIZE "FileSize"
#define LAUNCH_JOBKEY_RESOURCELIMIT_MEMLOCK "MemoryLock"
#define LAUNCH_JOBKEY_RESOURCELIMIT_NOFILE "NumberOfFiles"
#define LAUNCH_JOBKEY_RESOURCELIMIT_NPROC "NumberOfProcesses"
#define LAUNCH_JOBKEY_RESOURCELIMIT_RSS "ResidentSetSize"
#define LAUNCH_JOBKEY_RESOURCELIMIT_STACK "Stack"

#define LAUNCH_JOBKEY_DISABLED_MACHINETYPE "MachineType"
#define LAUNCH_JOBKEY_DISABLED_MODELNAME "ModelName"

#define LAUNCH_JOBSOCKETKEY_TYPE "SockType"
#define LAUNCH_JOBSOCKETKEY_PASSIVE "SockPassive"
#define LAUNCH_JOBSOCKETKEY_BONJOUR "Bonjour"
#define LAUNCH_JOBSOCKETKEY_SECUREWITHKEY "SecureSocketWithKey"
#define LAUNCH_JOBSOCKETKEY_PATHNAME "SockPathName"
#define LAUNCH_JOBSOCKETKEY_PATHMODE "SockPathMode"
#define LAUNCH_JOBSOCKETKEY_NODENAME "SockNodeName"
#define LAUNCH_JOBSOCKETKEY_SERVICENAME "SockServiceName"
#define LAUNCH_JOBSOCKETKEY_FAMILY "SockFamily"
#define LAUNCH_JOBSOCKETKEY_PROTOCOL "SockProtocol"
#define LAUNCH_JOBSOCKETKEY_MULTICASTGROUP "MulticastGroup"

/* These APIs are NOT suitable for general use. Their use should be constrained
 * to checking into launchd to obtain socket file descriptors using the
 * LAUNCH_CHECK_IN message type.
 */
typedef struct _launch_data *launch_data_t;

typedef enum {
	LAUNCH_DATA_DICTIONARY = 1,
	LAUNCH_DATA_ARRAY,
	LAUNCH_DATA_FD,
	LAUNCH_DATA_INTEGER,
	LAUNCH_DATA_REAL,
	LAUNCH_DATA_BOOL,
	LAUNCH_DATA_STRING,
	LAUNCH_DATA_OPAQUE,
	LAUNCH_DATA_ERRNO,
	LAUNCH_DATA_MACHPORT,
} launch_data_type_t;

__ld_allocator
launch_data_t
launch_data_alloc(launch_data_type_t);

__ld_allocator
launch_data_t
launch_data_copy(launch_data_t);

__ld_getter
launch_data_type_t
launch_data_get_type(const launch_data_t);

__ld_setter
void
launch_data_free(launch_data_t);

__ld_setter
bool
launch_data_dict_insert(launch_data_t, const launch_data_t, const char *);

__ld_getter
launch_data_t
launch_data_dict_lookup(const launch_data_t, const char *);

__ld_setter
bool
launch_data_dict_remove(launch_data_t, const char *);

__ld_iterator(1, 2)
void
launch_data_dict_iterate(const launch_data_t,
	void (*)(const launch_data_t, const char *, void *), void *);

__ld_getter
size_t
launch_data_dict_get_count(const launch_data_t);

__ld_setter
bool
launch_data_array_set_index(launch_data_t, const launch_data_t, size_t);

__ld_getter
launch_data_t
launch_data_array_get_index(const launch_data_t, size_t);

__ld_getter
size_t
launch_data_array_get_count(const launch_data_t);

__ld_allocator
launch_data_t
launch_data_new_fd(int);

__ld_allocator
launch_data_t
launch_data_new_machport(mach_port_t);

__ld_allocator
launch_data_t
launch_data_new_integer(long long);

__ld_allocator
launch_data_t
launch_data_new_bool(bool);

__ld_allocator
launch_data_t
launch_data_new_real(double);

__ld_allocator
launch_data_t
launch_data_new_string(const char *);

__ld_allocator
launch_data_t
launch_data_new_opaque(const void *, size_t);

 __ld_setter
bool
launch_data_set_fd(launch_data_t, int);

 __ld_setter
bool
launch_data_set_machport(launch_data_t, mach_port_t);

 __ld_setter
bool
launch_data_set_integer(launch_data_t, long long);

 __ld_setter
bool
launch_data_set_bool(launch_data_t, bool);

 __ld_setter
bool
launch_data_set_real(launch_data_t, double);

 __ld_setter
bool
launch_data_set_string(launch_data_t, const char *);

 __ld_setter
bool
launch_data_set_opaque(launch_data_t, const void *, size_t);

__ld_getter
int	
launch_data_get_fd(const launch_data_t);

__ld_getter
mach_port_t
launch_data_get_machport(const launch_data_t);

__ld_getter
long long
launch_data_get_integer(const launch_data_t);

__ld_getter
bool
launch_data_get_bool(const launch_data_t);

__ld_getter
double
launch_data_get_real(const launch_data_t);

__ld_getter
const char *
launch_data_get_string(const launch_data_t);

__ld_getter
void *
launch_data_get_opaque(const launch_data_t);

__ld_getter
size_t
launch_data_get_opaque_size(const launch_data_t);

__ld_getter
int
launch_data_get_errno(const launch_data_t);


/* launch_get_fd()
 *
 * Use this to get the FD if you're doing asynchronous I/O with select(),
 * poll() or kevent().
 */
__ld_normal
int
launch_get_fd(void);

/* launch_msg()
 *
 * Use this API to check in. Nothing else.
 */
__ld_normal
launch_data_t
launch_msg(const launch_data_t);

__END_DECLS

#pragma GCC visibility pop

#endif /* __LAUNCH_H__ */
