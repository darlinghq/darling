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

#include "AudioFileFormatManager.h"

AudioFileFormatManager::AudioFileFormatManager()
{

}

void AudioFileFormatManager::buildDatabase()
{
	AudioComponentDescription acd = { 0 };
	acd.componentType = 'afil';

	AudioComponent component = nullptr;

	while ((component = ::AudioComponentFindNext(component, &acd)) != nullptr)
	{
		AudioComponentInstance inst;
		OSStatus status = ::AudioComponentInstanceNew(component, &inst);

		if (status != noErr)
			continue;

		analyzeAudioFileComponent(inst);
		::AudioComponentInstanceDispose(inst);
	}
}

static std::vector<std::string> cfArrayToVector(CFArrayRef array)
{
	std::vector<std::string> rv;
	const CFIndex count = CFArrayGetCount(array);

	rv.reserve(count);

	for (CFIndex i = 0; i < count; i++)
	{
		CFStringRef str = (CFStringRef) CFArrayGetValueAtIndex(array, i);
		rv.push_back(CFStringGetCStringPtr(str, kCFStringEncodingUTF8));
	}

	return rv;
}

void AudioFileFormatManager::analyzeAudioFileComponent(AudioFileComponent component)
{
	ComponentInfo ci;
	UInt32 propSize;

	::AudioComponentGetDescription(AudioComponent(component), &ci.acd);

	ci.fileType = ci.acd.componentSubType; // TODO: Is this correct?

	{
		CFStringRef name = nullptr;
		propSize = sizeof(name);

		if (::AudioFileComponentGetGlobalInfo(component, kAudioFileComponent_FileTypeName, 0, nullptr, &propSize, &name) == noErr && name)
		{
			ci.name = CFStringGetCStringPtr(name, kCFStringEncodingUTF8);
			CFRelease(name);
		}
	}
	{
		UInt32 can;

		propSize = sizeof(can);
		ci.canRead = ci.canWrite = false;

		if (::AudioFileComponentGetGlobalInfo(component, kAudioFileComponent_CanRead, 0, nullptr, &propSize, &can) == noErr && can)
			ci.canRead = true;
		if (::AudioFileComponentGetGlobalInfo(component, kAudioFileComponent_CanWrite, 0, nullptr, &propSize, &can) == noErr && can)
			ci.canWrite = true;
	}

	CFArrayRef array = nullptr;
	if (::AudioFileComponentGetGlobalInfo(component, kAudioFileComponent_UTIsForType, 0, nullptr, &propSize, &array) == noErr && array)
	{
		ci.utis = cfArrayToVector(array);
		CFRelease(array);
	}

	array = nullptr;
	if (::AudioFileComponentGetGlobalInfo(component, kAudioFileComponent_MIMETypesForType, 0, nullptr, &propSize, &array) == noErr && array)
	{
		ci.mimeTypes = cfArrayToVector(array);
		CFRelease(array);
	}

	array = nullptr;
	if (::AudioFileComponentGetGlobalInfo(component, kAudioFileComponent_ExtensionsForType, 0, nullptr, &propSize, &array) == noErr && array)
	{
		ci.extensions = cfArrayToVector(array);
		CFRelease(array);
	}

	if (::AudioFileComponentGetGlobalInfoSize(component, kAudioFileComponent_AvailableFormatIDs, 0, nullptr, &propSize) == noErr)
	{
		const size_t formatCount = propSize / sizeof(UInt32);
		std::unique_ptr<UInt32[]> formatIds(new UInt32[formatCount]);

		if (::AudioFileComponentGetGlobalInfo(component, kAudioFileComponent_AvailableFormatIDs, 0, nullptr, &propSize, formatIds.get()) == noErr)
		{
			for (size_t i = 0; i < formatCount; i++)
			{
				UInt32 format = formatIds[i];
				std::vector<AudioStreamBasicDescription> asbds;
				
				if (::AudioFileComponentGetGlobalInfoSize(component, kAudioFileComponent_AvailableStreamDescriptionsForFormat, sizeof(UInt32), &format, &propSize) == noErr)
				{
					asbds.resize(propSize / sizeof(AudioStreamBasicDescription));
					
					if (::AudioFileComponentGetGlobalInfo(component, kAudioFileComponent_AvailableStreamDescriptionsForFormat, sizeof(UInt32), &format, &propSize, asbds.data()) != noErr)
						asbds.resize(0);

					ci.formatDescriptions.emplace(format, std::move(asbds));
				}
				else
					ci.formatDescriptions.emplace(format, asbds);
			}
		}
	}

	registerComponent(ci);
}

void AudioFileFormatManager::addToMap(std::unordered_map<std::string, std::vector<ComponentInfo*>>& map, const std::string& key, ComponentInfo* ci)
{
	auto it = map.find(key);
	if (it == map.end())
		it = map.emplace(key, std::vector<ComponentInfo*>()).first;
	it->second.push_back(ci);
}

void AudioFileFormatManager::registerComponent(const ComponentInfo& ci)
{
	m_components.push_back(ci);

	// Add format into various maps
	ComponentInfo* pci = &m_components.back();
	m_fileTypeMap.emplace(ci.fileType, pci);

	for (const std::string& uti : ci.utis)
		addToMap(m_utiMap, uti, pci);
	for (const std::string& mime : ci.mimeTypes)
		addToMap(m_mimeMap, mime, pci);
	for (const std::string& ext : ci.extensions)
		addToMap(m_extensionMap, ext, pci);
}

AudioFileFormatManager* AudioFileFormatManager::instance()
{
	static AudioFileFormatManager inst;
	return &inst;
}

std::set<UInt32> AudioFileFormatManager::availableFormatIDs(UInt32 fileType) const
{
	std::set<UInt32> rv;

	auto it = m_fileTypeMap.find(fileType);
	if (it == m_fileTypeMap.end())
		return rv;

	const ComponentInfo* ci = it->second;

	for (auto const& [key, value] : ci->formatDescriptions)
		rv.insert(key);
	return rv;
}

const AudioFileFormatManager::ComponentInfo* AudioFileFormatManager::fileType(UInt32 fileType) const
{
	auto it = m_fileTypeMap.find(fileType);
	if (it == m_fileTypeMap.end())
		return nullptr;
	return it->second;
}

std::set<UInt32> AudioFileFormatManager::types(bool writableTypes) const
{
	std::set<UInt32> rv;

	for (const ComponentInfo& ci : m_components)
	{
		if (writableTypes && ci.canWrite)
			rv.insert(ci.fileType);
		else if (!writableTypes && ci.canRead)
			rv.insert(ci.fileType);
	}

	return rv;
}

std::set<UInt32> AudioFileFormatManager::findTypes(const std::unordered_map<std::string, std::vector<ComponentInfo*>>& map, const char* key)
{
	std::set<UInt32> rv;

	auto it = map.find(key);
	if (it == map.end())
		return rv;
	
	for (const ComponentInfo* ci : it->second)
		rv.insert(ci->fileType);

	return rv;
}

std::set<UInt32> AudioFileFormatManager::typesForMIME(const char* mime) const
{
	return findTypes(m_mimeMap, mime);
}

std::set<UInt32> AudioFileFormatManager::typesForUTI(const char* uti) const
{
	return findTypes(m_utiMap, uti);
}

std::set<UInt32> AudioFileFormatManager::typesForExtension(const char* ext) const
{
	return findTypes(m_extensionMap, ext);
}

std::set<std::string> AudioFileFormatManager::allMIMEs() const
{
	return allKeys(m_mimeMap);
}

std::set<std::string> AudioFileFormatManager::allUTIs() const
{
	return allKeys(m_utiMap);
}

std::set<std::string> AudioFileFormatManager::allExtensions() const
{
	return allKeys(m_extensionMap);
}
