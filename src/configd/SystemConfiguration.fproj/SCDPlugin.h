/*
 * Copyright (c) 2000-2004, 2006, 2017 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _SCDPLUGIN_H
#define _SCDPLUGIN_H

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <CoreFoundation/CoreFoundation.h>


/*!
	@header SCDPlugin
 */


/*
	@defined kSCBundleRequiresKey
 */
#define kSCBundleRequiresKey		CFSTR("Requires")


/*
	@defined kSCBundleEnabledKey
 */
#define kSCBundleEnabledKey		CFSTR("Enabled")


/*
	@defined kSCBundleVerboseKey
 */
#define kSCBundleVerboseKey		CFSTR("Verbose")


/*
	@defined kSCBundleIsBuiltinKey
 */
#define kSCBundleIsBuiltinKey		CFSTR("Builtin")


/*!
	@typedef SCDynamicStoreBundleLoadFunction
	@discussion Type of the load() initialization function that will be
		called when a plug-in is loaded.  This function is called
		before calling the start() function and can be uesd to
		initialize any variables, open any sessions with "configd",
		and register any needed notifications.
	@param bundle The CFBundle being loaded.
	@param bundleVerbose A boolean value indicating whether verbose logging has
		been enabled for this bundle.
 */
typedef void	SCDynamicStoreBundleLoadFunction	(CFBundleRef	bundle,
							 Boolean	bundleVerbose);

/*!
	@typedef SCDynamicStoreBundleStartFunction
	@discussion Type of the start() initialization function that will be
		called after all plug-ins have been loaded and their load()
		functions have been called.  This function can initialize
		variables, open sessions with "configd", and register any
		needed notifications.
	@param bundleName The name of the plug-in / bundle.
	@param bundlePath The path name associated with the plug-in / bundle.
 */
typedef void	SCDynamicStoreBundleStartFunction	(const char	*bundleName,
							 const char	*bundlePath);

/*!
	@typedef SCDynamicStoreBundlePrimeFunction
	@discussion Type of the prime() initialization function that will be
		called after all plug-ins have executed their start() function but
		before the main plug-in run loop is started.  This function should
		be used to initialize any configuration information and/or state
		in the store.
 */
typedef void	SCDynamicStoreBundlePrimeFunction	(void);


/*!
	@typedef SCDynamicStoreBundleStopFunction
	@discussion Type of the stop() termination function that will be
		called when configd has been requested to shut down.
	@param stopRls A run loop source which should be signaled using
		CFRunLoopSourceSignal() when the plugin has been shut down.

	Note: a plugin can delay shut down of the daemon by no more than
		30 seconds.
 */
typedef void	SCDynamicStoreBundleStopFunction	(CFRunLoopSourceRef	stopRls);


/*!
	@typedef SCDPluginExecCallBack
	@discussion Type of the callback function used when a child process
		started by a plug-in has exited.
	@param pid The process id of the child which has exited.
	@param status The exit status of the child which has exited.
	@param rusage A summary of the resources used by the child process
		and all its children.
	@param context The callback argument specified on the call
		to _SCDPluginExecCommand().
 */
typedef void	(*SCDPluginExecCallBack)		(pid_t		pid,
							 int		status,
							 struct rusage	*rusage,
							 void		*context);


/*!
	@typedef SCDPluginExecSetup
	@discussion Type of the setup function used when a child process
		is being started.
	@param pid The process id of the child, zero for the child process.
	@param setupContext The setup argument specified on the call
		to _SCDPluginExecCommand2().
 */
typedef	void	(*SCDPluginExecSetup)			(pid_t		pid,
							 void		*setupContext);


__BEGIN_DECLS

/*!
	@function _SCDPluginExecCommand
	@discussion Starts a child process.
	@param callout The function to be called when the child
		process exits.  A NULL value can be specified if no
		callouts are desired.
	@param context A argument which will be passed
		to the callout function.
	@param uid The desired user id of the child process.
	@param gid The desired group id of the child process.
	@param path The command to be executed.
	@param argv The arguments to be passed to the child process.
	@result The process ID of the child.
 */
pid_t
_SCDPluginExecCommand		(
				SCDPluginExecCallBack	callout,
				void			*context,
				uid_t			uid,
				gid_t			gid,
				const char		*path,
				char * const 		argv[]
				);

/*!
	@function _SCDPluginExecCommand2
	@discussion Starts a child process.
	@param callout The function to be called when the child
		process exits.  A NULL value can be specified if no
		callouts are desired.
	@param context An argument which will be passed
		to the callout function.
	@param uid The desired user id of the child process.
	@param gid The desired group id of the child process.
	@param path The command to be executed.
	@param argv The arguments to be passed to the child process.
	@param setup A pointer to a function which, if specified, will
		be called after the call to fork(2) but before the call
		to exec(3).
		The function will be called in both the parent and child
		processes.
		The process ID returned by fork(2) will be passed as
		an argument to this function (i.e. non-zero in the parent,
		zero in the child).

		Note: the setup function is responsibile for establishing
		(and closing) all file descriptors that are (not) needed
		by the child process.
	@param setupContext An argument which will be passed
		to the setup function.
	@result The process ID of the child.
 */
pid_t
_SCDPluginExecCommand2		(
				SCDPluginExecCallBack	callout,
				void			*context,
				uid_t			uid,
				gid_t			gid,
				const char		*path,
				char * const 		argv[],
				SCDPluginExecSetup	setup,
				void			*setupContext
				);

__END_DECLS

#endif /* _SCDPLUGIN_H */
