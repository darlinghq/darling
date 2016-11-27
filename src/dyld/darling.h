/*
This file is part of Darling.

Copyright (C) 2016 Lubos Dolezel

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

#ifndef _DARLING_H_
#define _DARLING_H_

void showHelp(const char* argv0);

// Reports that this executable is not setuid root
void missingSetuidRoot(void);

// Returns ~/.darling with ~ expanded
char* defaultPrefixPath(void);
void setupWorkdir(const char* prefix);

// Creates the given directory, exit()ing if not possible
void createDir(const char* path);

// Return the PID of the init process in prefix (in our namespace)
// Returns 0 if no init is running
int getInitProcess(const char* prefix);

int spawnInitProcess(const char* prefix);

int darlingPreInit(void* arg);

void checkPrefixOwner(const char* prefix);

int isModuleLoaded(void);

int loadKernelModule(void);

#endif
