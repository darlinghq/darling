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

#include "darling-config.h"
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

void __prefix_set(const char* path) {}

const char* __prefix_get(void)
{
	return nullptr;
}

const char* translate_path_common(const char* path, bool symlink)
{
	return path;
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
	return path;
}

void __prefix_cwd(const char* in_path) {}

void __prefix_cwd_fd(int fd) {}

bool __prefix_is_system_root(const char* path)
{
	return strcmp(path, "/") == 0;
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
			real_path += comp;

			// Do NOT perform symlink resolution on /system-root,
			// because that should look like a bind mount rather than
			// a symlink from inside the DPREFIX
			bool is_system_root = false;

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
				char link[4096];
				int len;

				// std::cout << "*** readlink: " << real_path << std::endl;
				len = readlink(real_path.c_str(), link, sizeof(link)-1);

				if (len > 0)
				{
					std::list<std::string> link_components;

					link[len] = '\0';

					/*
					if (strncmp(link, "@darling_prefix@", 16) == 0)
					{
						std::string copy;

						copy = link;
						copy.replace(0, 16, INSTALL_PREFIX);

						strcpy(link, copy.c_str());
					}
					*/
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
	if (path.compare(0, sizeof(SHARE_PATH "/")-1, SHARE_PATH "/") == 0)
	{
		return path;
	}

	return real_path;
}
