#ifndef MACHOMGR_H
#define MACHOMGR_H
#include <stdint.h>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <util/rwmutex.h>
#if defined(__i386__) || defined(__x86_64__)
#	include "UndefinedFunction.h"
#	include "Trampoline.h"
#endif

struct mach_header;

namespace Darling {

class MachOObject;
class LoadableObject;
class NativeObject;

class MachOMgr
{
public:
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
	LoadableObject* lookup(const std::string& absolutePath);
	
	// Returns the main executable module - the one you can call run() on
	MachOObject* mainModule() { return m_mainModule; }
	
	// Searches all loaded modules for an exported symbol, prefering non-weak symbols
	void* getExportedSymbol(const std::string& symbolName, LoadableObject* nextAfter = nullptr);
	
	typedef void (LoaderHookFunc)(const struct ::mach_header* mh, intptr_t vmaddr_slide);
	
	// Register callbacks to be invoked when a image is loaded or unloaded
	void registerLoadHook(LoaderHookFunc* func);
	void registerUnloadHook(LoaderHookFunc* func);
	
	void deregisterLoadHook(LoaderHookFunc* func);
	void deregisterUnloadHook(LoaderHookFunc* func);
	
	// DYLD_BIND_AT_LAUNCH
	inline void setBindAtLaunch(bool bindAtLaunch) { m_bindAtLaunch = bindAtLaunch; }
	inline bool bindAtLaunch() const { return m_bindAtLaunch || useTrampolines(); }
	
	// DYLD_PRINT_INITIALIZERS
	inline void setPrintInitializers(bool printInitializers) { m_printInitializers = printInitializers; }
	inline bool printInitializers() const { return m_printInitializers; }
	
	// DYLD_PRINT_LIBRARIES
	inline void setPrintLibraries(bool printLibraries) { m_printLibraries = printLibraries; }
	inline bool printLibraries() const { return m_printLibraries; }
	
	// DYLD_LIBRARY_PATH
	inline void setLibraryPath(const std::string& paths) { m_libraryPath = paths; }
	inline const std::string& libraryPath() const { return m_libraryPath; }

	// Only call this method if you're loading a Mach-O library into a native process!
	void addDefaultLoader();

	// Darling specific: DYLD_TRAMPOLINES
	void setUseTrampolines(bool useTrampolines, const std::string& funcInfo);

	// Darling specific: DYLD_IGN_MISSING_SYMS
	void setIgnoreMissingSymbols(bool ignoreMissingSymbols);	
	
#ifdef HAS_DEBUG_HELPERS
	inline bool useTrampolines() const { return m_pTrampolineMgr != nullptr; }
	inline bool ignoreMissingSymbols() const { return m_pUndefMgr != nullptr; }
#else
	inline bool useTrampolines() const { return false; }
	inline bool ignoreMissingSymbols() const { return false; }
#endif

	// DYLD_PRINT_SEGMENTS
	inline void setPrintSegments(bool printSegments) { m_printSegments = printSegments; }
	inline bool printSegments() const { return m_printSegments; }

	// DYLD_PRINT_BINDINGS
	inline void setPrintBindings(bool printBindings) { m_printBindings = printBindings; }
	inline bool printBindings() const { return m_printBindings; }

	// DYLD_PRINT_RPATHS
	inline void setPrintRpathExpansion(bool print) { m_printRpathExpansion = print; }
	inline bool printRpathExpansion() const { return m_printRpathExpansion; }
	
	// Will enable loading of architectures that won't run on the current CPU.
	// Useful only for dyldd, not for general use.
	inline void setLoadAnyArchitecture(bool loadAny) { m_loadAny = loadAny; }
	inline bool loadAnyArchitecture() const { return m_loadAny; }
	
	bool detectSysRootFromPath(std::string path);
	inline void setSysRoot(const std::string& sysroot) { m_sysroot = sysroot; }
	inline const std::string& sysRoot() const { return m_sysroot; }
	inline bool hasSysRoot() const { return !m_sysroot.empty(); }
	
	inline size_t objectCount() const { return m_objectsInOrder.size(); }
	MachOObject* objectByIndex(size_t index);
	MachOObject* objectByHeader(struct mach_header* hdr);

	NativeObject* objectByNativeRef(void* nativeRef);

	void add(NativeObject* obj);
	void remove(NativeObject* obj);
	void notifyAdd(MachOObject* obj);
	
	static bool isTerminated() { return m_bTerminated; }
protected:
	friend class MachOObject;

#ifdef HAS_DEBUG_HELPERS
	inline UndefMgr* undefMgr() { return m_pUndefMgr; }
	inline TrampolineMgr* trampolineMgr() { return m_pTrampolineMgr; }
#endif

private:
	// map by base address
	std::map<void*, MachOObject*> m_objects;
	// map by absolute file name
	std::map<std::string, LoadableObject*> m_objectNames;
	// map by mach_header as needed by _dyld APIs
	std::map<struct mach_header*, MachOObject*> m_objectHeaders;
	// list with load order
	std::vector<MachOObject*> m_objectsInOrder;
	// this is needed for RTLD_NEXT only
	std::vector<LoadableObject*> m_loadablesInOrder;

	std::map<void*,NativeObject*> m_nativeRefToObject;
	
	mutable Darling::RWMutex m_lock;
	MachOObject* m_mainModule;
	bool m_bindAtLaunch, m_printInitializers, m_printLibraries;
	bool m_printSegments, m_printBindings, m_printRpathExpansion, m_loadAny;
	std::string m_libraryPath, m_sysroot;

#ifdef HAS_DEBUG_HELPERS
	UndefMgr* m_pUndefMgr = nullptr;
	TrampolineMgr* m_pTrampolineMgr = nullptr;
#endif
	
	std::set<LoaderHookFunc*> m_loadHooks, m_unloadHooks;
	bool m_addedDefaultLoader;
	static bool m_bTerminated;
};

} // namespace Darling

#endif
