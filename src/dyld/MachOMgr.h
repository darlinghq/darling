#ifndef MACHOMGR_H
#define MACHOMGR_H
#include <stdint.h>
#include <map>
#include <string>
#include <list>
#include <util/rwmutex.h>
#include "UndefinedFunction.h"
#include "Trampoline.h"

class MachOObject;

class MachOMgr
{
private:
	MachOMgr();
	~MachOMgr();
public:
	// Returns the singleton instance
	static MachOMgr* instance();
	
	// The maximum address used by any Mach-O mapping
	void* maxAddress() const;
	
	// Returns a MachOObject loaded at the given address
	MachOObject* objectForAddress(void* addr);
	
	// Registers a MachOObject with the manager
	void add(MachOObject* obj, bool mainModule = false);
	
	// Deregisters a MachOObject
	void remove(MachOObject* obj);
	
	// Returns a loaded MachOObject for a given absolute path
	MachOObject* lookup(const std::string& absolutePath);
	
	// Returns the main executable module - the one you can call run() on
	MachOObject* mainModule() { return m_mainModule; }
	
	// Searches all loaded modules for an exported symbol, prefering non-weak symbols
	void* getExportedSymbol(const std::string& symbolName);
	
	// DYLD_BIND_AT_LAUNCH
	inline void setBindAtLaunch(bool bindAtLaunch) { m_bindAtLaunch = bindAtLaunch; }
	inline bool bindAtLaunch() const { return m_bindAtLaunch || m_useTrampolines; }
	
	// DYLD_PRINT_INITIALIZERS
	inline void setPrintInitializers(bool printInitializers) { m_printInitializers = printInitializers; }
	inline bool printInitializers() const { return m_printInitializers; }
	
	// DYLD_PRINT_LIBRARIES
	inline void setPrintLibraries(bool printLibraries) { m_printLibraries = printLibraries; }
	inline bool printLibraries() const { return m_printLibraries; }
	
	// DYLD_LIBRARY_PATH
	inline void setLibraryPath(const std::string& paths) { m_libraryPath = paths; }
	inline const std::string& libraryPath() const { return m_libraryPath; }
	
	// Darling specific: DYLD_TRAMPOLINES
	inline void setUseTrampolines(bool useTrampolines) { m_useTrampolines = useTrampolines; }
	inline bool useTrampolines() const { return m_useTrampolines; }
	
	// Darling specific: DYLD_IGN_MISSING_SYMS
	inline void setIgnoreMissingSymbols(bool ignoreMissingSymbols) { m_ignoreMissingSymbols = ignoreMissingSymbols; }
	inline bool ignoreMissingSymbols() const { return m_ignoreMissingSymbols; }
	
	inline void setSysRoot(const std::string& sysroot) { m_sysroot = sysroot; }
	inline const std::string& sysRoot() const { return m_sysroot; }
private:
	// map by base address
	std::map<void*, MachOObject*> m_objects;
	// map by absolute file name
	std::map<std::string, MachOObject*> m_objectNames;
	// list with load order
	std::list<MachOObject*> m_objectsInOrder;
	
	mutable Darling::RWMutex m_lock;
	MachOObject* m_mainModule;
	bool m_bindAtLaunch, m_printInitializers, m_printLibraries, m_useTrampolines, m_ignoreMissingSymbols;
	std::string m_libraryPath, m_sysroot;
	
	UndefMgr* m_pUndefMgr;
	TrampolineMgr* m_pTrampolineMgr;
};

#endif
