/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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


#ifndef _AUDIO_FILE_FORMAT_MANAGER_H
#define _AUDIO_FILE_FORMAT_MANAGER_H
#include <AudioToolbox/AudioComponent.h>
#include <AudioToolbox/AudioFileComponent.h>
#include <vector>
#include <string>
#include <set>
#include <list>
#include <unordered_map>

class __attribute__((visibility("hidden"))) AudioFileFormatManager
{
public:
	static AudioFileFormatManager* instance();

	std::set<UInt32> availableFormatIDs(UInt32 fileType) const;

	struct ComponentInfo
	{
		AudioComponentDescription acd;
		UInt32 fileType; // container type
		std::string name;
		std::vector<std::string> utis, extensions, mimeTypes;
		// codec type
		std::unordered_map<UInt32, std::vector<AudioStreamBasicDescription>> formatDescriptions;
		bool canRead, canWrite;
	};

	const ComponentInfo* fileType(UInt32 type) const;
	std::set<UInt32> types(bool writableTypes) const;

	std::set<UInt32> typesForMIME(const char* mime) const;
	std::set<UInt32> typesForUTI(const char* uti) const;
	std::set<UInt32> typesForExtension(const char* ext) const;

	std::set<std::string> allMIMEs() const;
	std::set<std::string> allUTIs() const;
	std::set<std::string> allExtensions() const;
private:
	AudioFileFormatManager();
	void buildDatabase();
	void analyzeAudioFileComponent(AudioFileComponent component);
	void registerComponent(const ComponentInfo& ci);
	static void addToMap(std::unordered_map<std::string, std::vector<ComponentInfo*>>& map, const std::string& key, ComponentInfo* ci);
	static std::set<UInt32> findTypes(const std::unordered_map<std::string, std::vector<ComponentInfo*>>& map, const char* key);

	template <typename K, typename V>
	std::set<K> allKeys(const std::unordered_map<K,V>& map) const
	{
		std::set<K> rv;
		for (auto const& [k, v] : map)
			rv.insert(k);
		return rv;
	}
private:
	std::list<ComponentInfo> m_components;
	std::unordered_map<UInt32, ComponentInfo*> m_fileTypeMap;
	std::unordered_map<std::string, std::vector<ComponentInfo*>> m_mimeMap, m_utiMap, m_extensionMap;
};

#endif

