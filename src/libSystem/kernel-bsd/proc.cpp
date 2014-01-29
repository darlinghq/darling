/*
This file is part of Darling.

Copyright (C) 2013 Lubos Dolezel

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

#include "config.h"
#include "proc.h"
#include <fstream>
#include <sstream>
#include <string>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <ctype.h>
#include <string>
#include <memory>
#include "../libc/darwin_errno_codes.h"
#include "../libc/errno.h"

static std::string getProcStatElement(const char* pid, int elem);
static int sys_syslog(int type, char *bufp, int len); /* No wrapper provided in glibc */

int proc_name(int pid, void * buffer, uint32_t buffersize)
{
	std::stringstream path;
	std::ifstream procf;
	char procname[1024];
	size_t read;

	path << "/proc/" << pid << "/cmdline";
	procf.open(path.str().c_str());

	if (!procf.is_open())
	{
		errno = DARWIN_ENOENT;
		return 0;
	}

	procf.read(procname, sizeof procname);
	read = procf.gcount();

	for (size_t i = 0; i < read-1; i++)
	{
		if (!procname[i])
			procname[i] = ' ';
	}

	if (buffer)
	{
		if (buffersize >= read)
		{
			memcpy(buffer, procname, read);
			return read;
		}
		else
		{
			*((char*) buffer) = 0;
			return 0;
		}
	}

	return 0;
}

int proc_pidpath(int pid, void * buffer, uint32_t buffersize)
{
	std::stringstream path;
	char exe[1024];
	ssize_t len;

	path << "/proc/" << pid << "/exe";

	if (access(path.str().c_str(), F_OK) != 0)
	{
		errno = DARWIN_ENOENT;
		return 0;
	}

	len = readlink(path.str().c_str(), exe, sizeof(exe)-1);
	if (len < 0)
	{
		errnoOut();
		return 0;
	}
	exe[len] = 0;

	if (buffer && buffersize >= len+1)
	{
		memcpy(buffer, exe, len+1);
		return len+1;
	}

	return 0;
}

int proc_pidinfo(int pid, int flavor, uint64_t arg, void *buffer, int buffersize)
{
	// TODO
	return 0;
}

int proc_listpids(uint32_t type, uint32_t typeinfo, void *buffer, int buffersize)
{
	// buffer is an array of pid_t
	pid_t* pidOut = static_cast<pid_t*>(buffer);
	DIR* dirProc = opendir("/proc");
	const uid_t uid = getuid();
	int count = 0;

	if (!dirProc)
		return 0;

	while (struct dirent* ent = readdir(dirProc))
	{
		if (!isdigit(ent->d_name[0]))
			continue;

		bool pass;

		if (type == PROC_ALL_PIDS)
			pass = true;
		else if (type == PROC_UID_ONLY || type == PROC_RUID_ONLY) // not sure what to do with PROC_RUID_ONLY
		{
			char path[100];
			struct stat st;

			strcpy(path, "/proc/");
			strcat(path, ent->d_name);
			strcat(path, "/cmdline");

			if (stat(path, &st) == -1)
				continue;

			pass = st.st_uid == uid;
		}
		else if (type == PROC_PGRP_ONLY)
		{
			std::string grp = getProcStatElement(ent->d_name, 4);
			if (atoi(grp.c_str()) == typeinfo)
				pass = true;
		}
		else if (type == PROC_TTY_ONLY)
		{
			std::string grp = getProcStatElement(ent->d_name, 6);
			if (atoi(grp.c_str()) == typeinfo)
				pass = true;
		}
		else
			return 0;

		if (pidOut && count < buffersize / sizeof(pid_t))
			pidOut[count++] = atoi(ent->d_name);
	}

	closedir(dirProc);

	return count*sizeof(pid_t);
}

int proc_pidfdinfo(int pid, int fd, int flavor, void* buffer, int buffersize)
{
	// TODO
	return 0;
}

int proc_regionfilename(int pid, uint64_t address, void * buffer, uint32_t buffersize)
{
	std::ifstream file;
	std::string line;
	std::stringstream fname;

	fname << "/proc/" << pid << "/maps";
	file.open(fname.str().c_str());

	if (!file.is_open())
		return 0;
	
	while (std::getline(file, line))
	{
		if (line.empty()) continue;

		const char* s = line.c_str();
		uint64_t start, end;

		start = strtoll(s, (char**) &s, 16);
		s++;
		end = strtoll(s, (char**) &s, 16);
		
		if (address >= start && address < end)
		{
			if (line.size() > 74)
			{
				const char* file = line.c_str()+73;

				strncpy((char*) buffer, file, buffersize-1);
				*((char*) buffer + buffersize) = 0;

				return strlen((char*) buffer) + 1;
			}
			else
			{
				*((char*) buffer) = 0;
				return 1;
			}
		}
		
	}
	return 0;
}

int proc_kmsgbuf(void* buffer, uint32_t  buffersize)
{
	if (buffer)
	{
		int rv = sys_syslog(/*SYSLOG_ACTION_READ_ALL*/ 3, static_cast<char*>(buffer), buffersize);
		if (rv == -1)
		{
			errnoOut();
			return 0;
		}
		else
			return rv;
	}

	return 0;
}

int proc_libversion(int *major, int * minor)
{
	if (major)
		*major = 1;
	if (minor)
		*minor = 1;
	return 0;
}

int proc_setpcontrol(const int control)
{
	// TODO: specify options how to react on resource starvation
	// Doable on Linux?
	return 0;
}

int proc_listpidspath(uint32_t type, uint32_t typeInfo, const char* path, uint32_t pathFlags, void* buffer, int buffersize)
{
	// fuser-like functionality
	std::unique_ptr<pid_t[]> pids(new pid_t[4096]);
	int count = 0;
	int npids;
	dev_t devid;

	auto AddElem = [&](pid_t pid) -> void
	{
		if (buffer)
		{
			pid_t* p = static_cast<pid_t*>(buffer);
			if (buffersize/sizeof(pid_t) >= (count+1))
				p[count++] = pid;
		}
	};

	if (pathFlags & PROC_LISTPIDSPATH_PATH_IS_VOLUME)
	{
		struct stat st;
		if (stat(path, &st) == -1)
			return 0;
		devid = st.st_dev;
	}

	npids = proc_listpids(type, typeInfo, pids.get(), 4096*sizeof(pid_t));

	for (int i = 0; i < npids/sizeof(pid_t); i++)
	{
		char dpath[100];
		DIR* dir;
		struct dirent* ent;
		int orig;

		sprintf(dpath, "/proc/%d/fd/", pids[i]);
		orig = strlen(dpath);
		dir = opendir(dpath);

		if (!dir)
			continue;

		while ((ent = readdir(dir)))
		{
			if (ent->d_name[0] == '.')
				continue;
			
			strcat(dpath, ent->d_name);

			if (pathFlags & PROC_LISTPIDSPATH_PATH_IS_VOLUME) // like `fuser -m`
			{
				struct stat st;
				int rv;

				rv = stat(dpath, &st);
				dpath[orig] = 0;

				if (rv == -1)
					continue;

				if (st.st_dev == devid)
					AddElem(pids[i]);
			}
			else // like `fuser`
			{
				char lpath[PATH_MAX];
				ssize_t bytes;

				bytes = readlink(dpath, lpath, sizeof(lpath));
				dpath[orig] = 0;
	
				if (bytes < 0)
					continue;
				
				lpath[bytes] = 0;

				if (strcmp(lpath, path) == 0)
					AddElem(pids[i]);
			}
		}

		closedir(dir);
	}

	return count*sizeof(pid_t);
}

std::string getProcStatElement(const char* pid, int elem)
{
	char path[100] = "/proc/";
	std::ifstream file;
	std::string result;

	strcat(path, pid);
	strcat(path, "/stat");

	file.open(path);

	if (!file.is_open())
		return std::string();

	for (int p = 0; p <= elem; p++)
	{
		int par = 0;
		while (true)
		{
			int c = file.get();
			if (!par && c == ' ')
				break;
			if (par == 2 && c == ')')
			{
				file.get();
				break;
			}
			if (!par && c == '(')
				par = 2;
			else
			{
				par = 1;
				result += char(c);
			}
		}
	}

	return result;
}

int sys_syslog(int type, char *bufp, int len)
{
	return syscall(SYS_syslog, type, bufp, len);
}

