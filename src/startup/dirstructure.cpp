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

#include "dirstructure.h"
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

static std::string GetUserLibrary()
{
	const char *home, *prefix;
	std::stringstream ss;
	std::string path;

	home = getenv("HOME");
	if (!home)
		return std::string(); // give up on this user

	ss << home << '/' << "Library" << '/';
	return ss.str();
}

bool HasUserDirectoryStructure()
{
	std::string path = GetUserLibrary();

	if (path.empty())
		return true; // give up on this user

	if (::access(path.c_str(), F_OK) == -1)
		return false;
	else
		return true;
}

void SetupUserDirectoryStructure()
{
	const char* dirs[] = {
		"Application Support",
		"Assistants",
		"Audio",
		"Audio/MIDI Drivers",
		"Audio/Plug-Ins",
		"Audio/Plug-Ins/Components",
		"Audio/Plug-Ins/Digidesign",
		"Audio/Plug-Ins/VST",
		"Audio/Sounds",
		"Audio/Sounds/Alerts",
		"Audio/Sounds/Banks",
		"Caches",
		"ColorPickers",
		"Colors",
		"Compositions",
		"Favorites",
		"FontCollections",
		"Fonts",
		"iMovie",
		"iMovie/Plug-ins",
		"iMovie/Sound Effects",
		"Input Methods",
		"Internet Plug-Ins",
		"Keyboard Layouts",
		"Logs",
		"PreferencePanes",
		"Preferences",
		"Printers",
		"Screen Savers",
		"Sounds",
		"Spelling",
		"Voices",
	};

	std::string path = GetUserLibrary();

	if (path.empty())
		return;

	std::cerr << "Darling: Creating Library structure at " << path << std::endl;

	if (::mkdir(path.c_str(), 0777) == -1)
		std::cerr << "Darling: Cannot mkdir(" << path << "): " << strerror(errno) << std::endl;

	for (const char* dir : dirs)
	{
		std::string s = path;

		s += dir;

		if (::mkdir(s.c_str(), 0777) == -1)
			std::cerr << "Darling: Cannot mkdir(" << s << "): " << strerror(errno) << std::endl;
	}
}
