#include "MachOMgr.h"
#include <cassert>
#include <algorithm>
#include "MachOObject.h"

MachOMgr::MachOMgr()
: m_mainModule(nullptr), m_bindAtLaunch(false), m_printInitializers(false),
  m_printLibraries(false), m_useTrampolines(false), m_ignoreMissingSymbols(false)
{
}

MachOMgr::~MachOMgr()
{
	while (!m_objects.empty())
	{
		MachOObject* obj = m_objectsInOrder.front();
		obj->unload();
	}
}

MachOMgr* MachOMgr::instance()
{
	static MachOMgr obj;
	return &obj;
}

void* MachOMgr::maxAddress() const
{
	Darling::RWMutexReadLock l(m_lock);
	
	if (m_objects.empty())
		return 0;
	else
	{
		auto it = m_objects.end();
		it--;
		return it->second->maxAddress();
	}
}

MachOObject* MachOMgr::objectForAddress(void* addr)
{
	Darling::RWMutexReadLock l(m_lock);
	
	auto it = m_objects.upper_bound(addr);
	
	if (it == m_objects.begin() || it == m_objects.end())
		return nullptr;
	
	it--;
	return it->second;
}

void MachOMgr::add(MachOObject* obj, bool mainModule)
{
	Darling::RWMutexWriteLock l(m_lock);
	
	m_objects[obj->baseAddress()] = obj;
	m_objectNames[obj->path()] = obj;
	m_objectsInOrder.push_back(obj);
	
	if (mainModule)
	{
		assert(m_mainModule == nullptr);
		m_mainModule = obj;
	}
}

template <typename Key, typename Value> void mapEraseByValue(std::map<Key, Value>& map, const Value& v)
{
	for (auto it = map.begin(); it != map.end(); it++)
	{
		if (it->second == v)
		{
			map.erase(it);
			break;
		}
	}
}

void MachOMgr::remove(MachOObject* obj)
{
	Darling::RWMutexWriteLock l(m_lock);
	
	// slow!
	mapEraseByValue(m_objects, obj);
	mapEraseByValue(m_objectNames, obj);
	
	auto it = std::find(m_objectsInOrder.begin(), m_objectsInOrder.end(), obj);
	if (it != m_objectsInOrder.end())
		m_objectsInOrder.erase(it);
	
	if (m_mainModule == obj)
		m_mainModule = nullptr;
}

void* MachOMgr::getExportedSymbol(const std::string& symbolName)
{
	for (MachOObject* obj : m_objectsInOrder)
	{
		void* p = obj->getExportedSymbol(symbolName);
		if (p)
			return p;
	}
	return nullptr;
}
