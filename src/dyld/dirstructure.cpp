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
	const char* home;
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

