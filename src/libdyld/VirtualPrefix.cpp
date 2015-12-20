/*
This file is part of Darling.

Copyright (C) 2015 Lubos Dolezel

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

#include "VirtualPrefix.h"
#include <string>
#include <cstring>
#include <pthread.h>
#include <list>
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <cstdio>

static std::string g_prefix;
static std::string g_cwd;
static pthread_rwlock_t g_cwdLock = PTHREAD_RWLOCK_INITIALIZER;
static const char SYSTEM_ROOT[] = __SYSTEM_ROOT;

static std::string canonicalize_path(const std::string& in_path);

void __prefix_set(const char* path)
{
	char cwd[256];
	
	getcwd(cwd, sizeof(cwd));
	
	g_prefix = path;
	if (g_prefix[g_prefix.length()-1] != '/')
		g_prefix += '/';
	
	if (strncmp(cwd, path, strlen(path)) == 0)
	{
		g_cwd = path + strlen(path);
		if (g_cwd.empty())
			g_cwd = "/";
	}
	else
	{
		g_cwd = SYSTEM_ROOT;
		g_cwd += cwd;
		g_cwd += '/';
	}
}

const char* __prefix_get(void)
{
	if (g_prefix.empty())
		return nullptr;
	else
		return g_prefix.c_str();
}

const char* __prefix_translate_path(const char* path)
{
	static thread_local char resolved_path[1024];
	std::string str;
	
	if (g_prefix.empty())
		return path;
	
	if (path[0] != '/')
	{
		pthread_rwlock_rdlock(&g_cwdLock);
		str = g_cwd;
		pthread_rwlock_unlock(&g_cwdLock);
	}
	str += path;
	
	// Resolve . and ..
	str = canonicalize_path(str);
	
	// std::cout << "CWD: " << g_cwd << std::endl;
	// std::cout << "Can1: " << path << " -> " << str << std::endl;
	if (str.compare(0, sizeof(SYSTEM_ROOT)-1, SYSTEM_ROOT) == 0)
	{
		// Leave virtual prefix
		str = str.substr(sizeof(SYSTEM_ROOT)-1);
		if (str.empty())
			str = "/";
	}
	else if (str.compare(0, 5, "/proc") != 0)
	{
		// Apply virtual prefix
		str = g_prefix + str;
	}
	
	// std::cout << "Can2: " << path << " -> " << str << std::endl;
	
	// TODO: make case insensitive
	
	strncpy(resolved_path, str.c_str(), sizeof(resolved_path)-1);
	resolved_path[sizeof(resolved_path)-1] = '\0';
	
	return resolved_path;
}

const char* __prefix_untranslate_path(const char* path, unsigned long count)
{
	static thread_local char resolved_path[1024];
	size_t test_len;
	
	// FIXME: The following strcmp is a bit of a hack needed for isatty()
	// and friends.
	if (g_prefix.empty() || !count || path[0] != '/'
			|| strncmp(path, "/dev/", 5) == 0)
	{
		memcpy(resolved_path, path, count);
		resolved_path[count] = '\0';
		return resolved_path;
	}
	
	test_len = (path[count-1] != '/') ? (g_prefix.length()-1)
			: (g_prefix.length());
	
	if (strncmp(path, g_prefix.c_str(), test_len) == 0)
	{
		size_t len = count - (g_prefix.length()-1);
		memcpy(resolved_path, path + g_prefix.length() - 1,
				count - (g_prefix.length() - 1));
		
		if (len > 0)
			resolved_path[len] = '\0';
		else
			strcpy(resolved_path, "/");
	}
	else
	{
		size_t len = std::min<size_t>(sizeof(resolved_path)
				- sizeof(SYSTEM_ROOT), count);
		
		strcpy(resolved_path, SYSTEM_ROOT);
		strncat(resolved_path, path, len);
		resolved_path[len + sizeof(SYSTEM_ROOT)-1] = '\0';
	}
	
	return resolved_path;
}

void __prefix_cwd(const char* in_path)
{
	if (!*in_path)
		return;

	std::string path;
	
	// std::cout << "CWD to " << in_path << std::endl;
	
	pthread_rwlock_wrlock(&g_cwdLock);
	if (in_path[0] != '/')
	{
		path = g_cwd;
		path += in_path;
	}
	else
		path = in_path;
	
	g_cwd = canonicalize_path(path);
	
	if (g_cwd[g_cwd.length()-1] != '/')
		g_cwd += '/';
	
	pthread_rwlock_unlock(&g_cwdLock);
}

void __prefix_cwd_fd(int fd)
{
	char path[1024];
	int count;
	
	if (g_prefix.empty())
		return;
	
	sprintf(path, "/proc/self/fd/%d", fd);
	count = readlink(path, path, sizeof(path)-1);
	
	if (count < 0)
		return;
	
	__prefix_cwd(__prefix_untranslate_path(path, count));
}

bool __prefix_is_system_root(const char* path)
{
	return strcmp(__prefix_translate_path(path), "/") == 0;
}

int __prefix_get_dyld_path(char* buf, unsigned long size)
{
	int len;
	len = readlink("/proc/self/exe", buf, size);
	
	if (len < 0)
		buf[0] = '\0';
	else
		buf[len] = '\0';
	
	return len;
}

std::string canonicalize_path(const std::string& in_path)
{
	std::list<std::string> path_components;
	size_t pos = 0, last_pos;
	std::string path;
	
	assert(in_path[0] == '/');
	
	while (pos != std::string::npos)
	{
		size_t len;
		last_pos = pos + 1;
		
		pos = in_path.find('/', last_pos);
		len = (pos == std::string::npos) ? pos : (pos - last_pos);
		path_components.push_back(in_path.substr(last_pos, len));
	}
	
	for (std::list<std::string>::iterator it = path_components.begin();
			it != path_components.end(); )
	{
		if (*it == "." || it->empty())
		{
			it = path_components.erase(it);
		}
		else if (*it == "..")
		{
			if (it != path_components.begin())
			{
				it--;
				it = path_components.erase(it);
			}
			it = path_components.erase(it);
		}
		else
			it++;
	}
	
	path.reserve(in_path.size());
	
	for (const std::string& comp : path_components)
	{
		path += '/';
		path += comp;
	}
	
	return path;
}
