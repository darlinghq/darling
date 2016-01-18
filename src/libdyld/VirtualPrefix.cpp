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
#include <sys/stat.h>
#include <iostream>
#include <cstdio>
#include <dirent.h>

static std::string g_prefix;
static std::list<std::string> g_prefixComponents;
static std::string g_cwd;
static pthread_rwlock_t g_cwdLock = PTHREAD_RWLOCK_INITIALIZER;
static const char SYSTEM_ROOT[] = __SYSTEM_ROOT;

static std::list<std::string> explode_path(const std::string& path);
static std::string join_path(const std::list<std::string>& path_components);

static std::list<std::string>& canonicalize_path(std::list<std::string>& path_components);
static std::string resolve_path(std::list<std::string>& path_components, bool symlink);
static const char* translate_path_common(const char* path, bool symlink);

void __prefix_set(const char* path)
{
	char cwd[256];
	size_t pos = 0, last_pos;
	
	getcwd(cwd, sizeof(cwd));
	
	assert(path[0] == '/');
	assert(g_prefix.empty());
	
	g_prefix = path;
	g_prefixComponents.clear();
	
	while (pos != std::string::npos)
	{
		size_t len;
		last_pos = pos + 1;
		
		pos = g_prefix.find('/', last_pos);
		len = (pos == std::string::npos) ? pos : (pos - last_pos);
		g_prefixComponents.push_back(g_prefix.substr(last_pos, len));
	}
	
	if (g_prefix[g_prefix.length()-1] != '/')
		g_prefix += '/';
	
	if (strncmp(cwd, path, strlen(path)) == 0)
	{
		g_cwd = cwd + strlen(path);
		if (g_cwd.empty() || g_cwd[g_cwd.length()-1] != '/')
			g_cwd += "/";
	}
	else
	{
		g_cwd = SYSTEM_ROOT;
		g_cwd += cwd;
		g_cwd += '/';
	}
	
	// std::cout << "### Prefix initialized with cwd " << g_cwd << " from " << cwd << std::endl;
}

const char* __prefix_get(void)
{
	if (g_prefix.empty())
		return nullptr;
	else
		return g_prefix.c_str();
}

const char* translate_path_common(const char* path, bool symlink)
{
	static thread_local char resolved_path[1024];
	std::string str;
	std::list<std::string> path_components;
	
	if (g_prefix.empty())
		return path;
	
	// std::cout << "\tCWD is " << g_cwd << std::endl;
	
	if (path[0] != '/')
	{
		pthread_rwlock_rdlock(&g_cwdLock);
		str = g_cwd;
		pthread_rwlock_unlock(&g_cwdLock);
	}
	str += path;
	// std::cout << "*** Before explode: " << str << std::endl;
	
	path_components = explode_path(str);
	str = resolve_path(path_components, symlink);
	
	strncpy(resolved_path, str.c_str(), sizeof(resolved_path)-1);
	resolved_path[sizeof(resolved_path)-1] = '\0';
	
	// std::cout << "*** In: " << path << "; out: " << resolved_path << std::endl;
	
	return resolved_path;
}

const char* __prefix_translate_path(const char* path)
{
	return translate_path_common(path, false);
}

const char* __prefix_translate_path_link(const char* path)
{
	return translate_path_common(path, true);
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
	
	// std::cout << "*** UNTRANSLATE: " << path << " -> " << resolved_path << std::endl;
	
	return resolved_path;
}

void __prefix_cwd(const char* in_path)
{
	if (!*in_path)
		return;

	std::string path;
	std::list<std::string> path_components;
	
	// std::cout << "CWD to " << in_path << std::endl;
	
	pthread_rwlock_wrlock(&g_cwdLock);
	if (in_path[0] != '/')
	{
		path = g_cwd;
		path += in_path;
	}
	else
		path = in_path;
	
	path_components = explode_path(path);
	g_cwd = join_path(canonicalize_path(path_components));
	
	if (g_cwd[g_cwd.length()-1] != '/')
		g_cwd += '/';
	
	// std::cout << "\t+++ CWD In: " << in_path << "; out: " << g_cwd << std::endl;
	
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

std::list<std::string> explode_path(const std::string& path)
{
	std::list<std::string> path_components;
	size_t pos = 0, last_pos;
	
	if (path.empty())
		return path_components;
	
	while (pos != std::string::npos)
	{
		size_t len;
		
		if (pos != 0 || path[0] == '/')
			last_pos = pos + 1; // skip first slash
		else
			last_pos = pos;
		
		pos = path.find('/', last_pos);
		len = (pos == std::string::npos) ? pos : (pos - last_pos);
		path_components.push_back(path.substr(last_pos, len));
	}
	
	return path_components;
}

std::string join_path(const std::list<std::string>& path_components)
{
	std::string path;
	
	for (const std::string& comp : path_components)
	{
		path += '/';
		path += comp;
	}
	
	return path;
}

std::list<std::string>& canonicalize_path(std::list<std::string>& path_components)
{
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
	
	return path_components;
}

std::string resolve_path(std::list<std::string>& path_components, bool symlink)
{
	std::string path, real_path;
	bool had_failure = false;
	
	real_path = g_prefix;
	
	// The resolution process is restarted when a symlink is encountered
restart_process:
			
	canonicalize_path(path_components);
	path.clear();
	
	for (std::list<std::string>::iterator it = path_components.begin();
			it != path_components.end(); it++)
	{
		std::string& comp = *it;
		
		path += '/';
		
		if (!had_failure) // check if there is any sense in using opendir() or lstat() again
		{
			DIR* dir;
			struct dirent* ent;
			struct stat st;
			bool isLink = false;
			bool is_system_root;
			
			real_path.replace(g_prefix.size(), std::string::npos, path);
			real_path += comp;
			
			// Do NOT perform symlink resolution on /system-root,
			// because that should look like a bind mount rather than
			// a symlink from inside the DPREFIX
			is_system_root = (it == path_components.begin())
						&& (comp == (SYSTEM_ROOT+1));
			
			// We try an lstat() first as an optimization,
			// because the opendir() path below is VERY slow on NFS mounts.
			if (lstat(real_path.c_str(), &st) == 0)
			{
				isLink = S_ISLNK(st.st_mode);
			}
			else
			{
				// std::cout << "*** opendir: " << real_path << std::endl;
				real_path.resize(real_path.length() - comp.length());
				
				dir = opendir(real_path.c_str());

				if (dir != nullptr)
				{
					std::string best_match;
					unsigned char best_match_type;

					while ((ent = readdir(dir)) != nullptr)
					{
						// Commented out: replaced with lstat() above
						/* if (comp == ent->d_name)
							break;
						else*/ if (strcasecmp(comp.c_str(), ent->d_name) == 0)
						{
							best_match = ent->d_name;
							best_match_type = ent->d_type;
						}
					}

					if (ent != nullptr || !best_match.empty())
					{
						if (ent == nullptr)
						{
							// correct the case
							comp = best_match;
						}
						else
							best_match_type = ent->d_type;

						if (best_match_type == DT_LNK && !is_system_root)
						{
							isLink = true;
							real_path += comp;
						}
					}
					else
						had_failure = true;

					closedir(dir);
				}
				else
					had_failure = true;
			}
			
			if (symlink && it == --path_components.end())
				isLink = false;
			
			// Perform symlink resolution
			if (isLink && !is_system_root)
			{
				char link[256];
				int len;

				// std::cout << "*** readlink: " << real_path << std::endl;
				len = readlink(real_path.c_str(), link, sizeof(link)-1);

				if (len > 0)
				{
					std::list<std::string> link_components;

					link[len] = '\0';

					link_components = explode_path(link);

					if (link[0] == '/')
					{
						// absolute symlink
						it++;
						it = path_components.erase(path_components.begin(), it);
					}
					else
					{
						// relative symlink
						it = path_components.erase(it);
					}

					path_components.insert(it,
								link_components.begin(),
								link_components.end());

					// We always restart the process
					// 1) For absolute symlinks, because we're moving to a completely different path
					// 2) For relative symlinks, because they may contain '..'
					goto restart_process;
				}
			}
		}
		
		path += comp;
	}
	
	if (!path_components.empty())
	{
		if (*path_components.begin() == "proc")
		{
			return path;
		}
		else if (*path_components.begin() == (SYSTEM_ROOT+1))
		{
			// Exit virtual prefix
			return path.substr(sizeof(SYSTEM_ROOT)-1);
		}
	}
	
	// Apply virtual prefix
	real_path.replace(g_prefix.size(), std::string::npos, path);
	return real_path;
}

/*
void path_to_string(const std::list<std::string>& path, char* outPath,
		size_t maxLen)
{
	size_t lenSoFar = 1;
	
	if (maxLen < 2)
		return;
	
	outPath[0] = '\0';
	
	for (const std::string& comp : path)
	{
		if (lenSoFar+1 > maxLen)
			break;
		
		strcat(outPath, "/");
		lenSoFar++;
		
		if (lenSoFar + comp.length() > maxLen)
		{
			strncat(outPath, comp.c_str(), maxLen - lenSoFar);
			break;
		}
		else
		{
			strcat(outPath, comp.c_str());
			lenSoFar += comp.length();
		}
	}
}
*/
