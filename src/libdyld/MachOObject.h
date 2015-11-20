#ifndef MACHOOBJECT_H
#define MACHOOBJECT_H
#include <libmach-o/MachO.h>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <stdint.h>
#include <climits>
#include "LoadableObject.h"

// friends

namespace Darling {

struct ProgramVars;

class MachOObject : public LoadableObject
{
public:
	MachOObject(const std::string& path);
	
	// takes ownership of `file'
	MachOObject(MachO* file);
	
	~MachOObject();
	
	inline bool valid() const { return m_file != nullptr; }
	
	// Absolute path to this object
	virtual const std::string& path() const override { return m_absolutePath; }
	
	// The directory that contains this object
	inline const std::string& directory() const { return m_absolutePathDir; }

	// Some name identifying this object
	virtual const char* name() const override;
	
	// maximum address mapped by this object
	void* maxAddress() const;
	
	// minimum address mapped by this object
	void* baseAddress() const { return m_base; }
	
	// The difference between addresses in the MachO file and actually mapped addresses.
	// For libraries, slide() == baseAddress()
	uintptr_t slide() const { return m_slide; }
	
	// Loads the Mach-O object into memory
	// Needs to be called before using almost all other functions
	virtual void load() override;

	// Unloads the Mach-O object from memory
	// Images that are not unloaded manually will be unloaded automatically
	// upon the destruction of MachOMgr
	virtual void unload() override;

	virtual bool isLoaded() const override;

	virtual void* getExportedSymbol(const std::string& symbolName, bool nonWeakOnly) const override;

	virtual void atExit() override;

	void* getExportedSymbolRecursive(const std::string& symbolName) const;

	// Fills a Dl_info structure in a dladdr() manner
	virtual bool findSymbolInfo(const void* addr, Dl_info* p) const override;
	
	// Returns true if this is a main module. Main module is an executable (as opposed to dynamic libraries)
	bool isMainModule() const;
	
	// Set the command line arguments before using run() to start the program
	// Only applicable to main modules.
	void setCommandLine(int argc, char** argv, char** envp);
	void commandLine(int* argc, char*** argv, char*** envp);
	
	// Should not be normally used
	void commandLine(int** argc, char**** argv, char**** envp);
	
	// Executes the application. Use setCommandLine() first.
	// Only possible on the main module. This method either never returns or throws an exception
	__attribute__((noreturn))
	void run();
	
	// Returns a pointer to the beginning of a Mach-O section, identified by the segment and section name
	// Segment name can be left empty
	void* getSection(const std::string& segmentName, const std::string& sectionName, uintptr_t* sectionSize = nullptr);
	
	// Returns the header of the Mach-O object
	inline struct ::mach_header* getMachHeader() { return &m_header; }

	// For RTLD_FIRST implementation, not useful outside of dl_public
	inline void setNoRecursion(bool noRecursion) { m_noRecursion = noRecursion; }
	inline bool noRecursion() const { return m_noRecursion; }
	
	std::vector<const char*> declaredDependencies() const { return m_file->dylibs(); }
	
protected:
	friend class DylibSearch;
	friend class LoadableObject;
	
	void setRequesterRunpaths(MachOObject* requester);
	const std::vector<std::string>& runPaths() const { return m_rpaths; }
	
	ProgramVars* getProgramVars();
private:
	void postConstruct();
	
	void loadSegments();
	void unloadSegments();
	void setInitialSegmentProtection();
	void readSymbols();
	void readExports();

	static std::string expandLoaderPath(std::string path, MachOObject* loader);
	
	void rebase();
	
	void runInitializers();
	void runFinalizers();
	
	void loadDependencies();
	void fillInProgramVars();
	void fillInDyldData();
	
	void registerEHSection();
	void unregisterEHSection();
	
	void performRelocations();
	void performBinds();
	void* performBind(MachO::Bind* bind);
	
	void detectAbsolutePath();
	void* resolveSymbol(const std::string& name, int libraryOrdinal);
	
	void jumpToStart() __attribute__((noreturn));
	
	void setupTLS();
	void teardownTLS();

	void* getExportedSymbolRecursive(const std::string& symbolName, std::set<const LoadableObject*>& visited) const;
	
	static void* dyld_stub_binder_fixup(MachOObject** obj, uintptr_t lazyOffset) asm("dyld_stub_binder_fixup");
	
	// Invoked by a lazy-binding stub
	void* doLazyBind(uintptr_t lazyOffset);
	
	static int machoProtectionFlagsToMmap(int machoFlags);
	static uintptr_t pageAlign(uintptr_t what);
	static uintptr_t getMinMappingAddr();
	static void writeBind(int type, void** ptr, void* newAddr, const std::string& name);
	static bool lookupDyldFunction(const char* name, void** addr);

	uintptr_t getTotalMappingSize();

	inline bool usesTwoLevelNamespace() const { return m_header.flags & MH_TWOLEVEL; }
private:
	MachO* m_file;
	int m_refs = 1;
	void* m_base = nullptr;
	uintptr_t m_slide = 0;
	
	std::vector<void*> m_initializers, m_finalizers;
	mutable std::unordered_map<std::string, MachO::Export> m_exports; // mutable because of 'resolver' functions
	std::map<void*, const char*> m_symbols;
	std::vector<LoadableObject*> m_dependencies;
	
	struct Mapping
	{
		void* start;
		uintptr_t size;
		int initprot, maxprot;
	};
	std::vector<Mapping> m_mappings;
	std::vector<std::string> m_rpaths;
	void* m_reworkedEHData = nullptr;
	
	std::string m_absolutePath, m_absolutePathDir, m_name;
	
	::mach_header m_header;
	
	int m_argc = 0;
	char** m_argv = nullptr;
	char** m_envp = nullptr;
	bool m_noRecursion = false;
};

} // namespace Darling

#endif
