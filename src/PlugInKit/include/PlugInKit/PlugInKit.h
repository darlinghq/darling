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

#import <Foundation/Foundation.h>

void* oslog(void);
void* oslog_ext(void);
void* oslog_ls(void);
void* oslog_traffic(void);
void* pkAugmentInterface(void);
void* pkError(void);
void* pkErrorf(void);
void* pkFindProtocol(void);
void* pkFindProtocols(void);
void* pkIssueSandboxExtension(void);
void* xpc_dictionary_get_array(void);
void* xpc_dictionary_get_dictionary(void);

#import <PlugInKit/PKHostPlugIn.h>
#import <PlugInKit/PKDiscoveryDriver.h>
#import <PlugInKit/PKDiscoveryLSWatcher.h>
#import <PlugInKit/PKHost.h>
#import <PlugInKit/PKPlugInCore.h>
#import <PlugInKit/PKServicePersonality.h>
#import <PlugInKit/PKService.h>
#import <PlugInKit/PKHostDefaults.h>
#import <PlugInKit/PKServiceDefaults.h>
#import <PlugInKit/PKDaemonClient.h>
#import <PlugInKit/PKBundle.h>
#import <PlugInKit/PKSandboxExtension.h>
#import <PlugInKit/PKXPCObject.h>
#import <PlugInKit/PKManager.h>
