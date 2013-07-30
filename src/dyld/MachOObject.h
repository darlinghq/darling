#ifndef MACHOOBJECT_H
#define MACHOOBJECT_H
#include <libmach-o/MachO.h>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <stdint.h>
#include <climits>
#include "LoadableObject.h"

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
	inline const std::string& path() const { return m_absolutePath; }
	
	// The directory that contains this object
	inline const std::string& directory() const { return m_absolutePathDir; }
	
	// maximum address mapped by this object
	void* maxAddress() const;
	
	// minimum address mapped by this object
	void* baseAddress() const { return m_base; }
	
	// The difference between addresses in the MachO file and actually mapped addresses.
	// For libraries, slide() == baseAddress()
	uintptr_t slide() const { return m_slide; }
	
	virtual void load() override;
	virtual void unload() override;
	
	virtual void* getExportedSymbol(const std::string& symbolName, bool nonWeakOnly) const override;
	virtual bool findSymbolInfo(const void* addr, Dl_info* p) const override;
	
	bool isMainModule() const;
	
	void setCommandLine(int argc, char** argv, char** envp);
	void commandLine(int* argc, char*** argv, char*** envp);
	
	// Only possible on the main module
	void run();
	
	void* getSection(const std::string& segmentName, const std::string& sectionName, uintptr_t* sectionSize = nullptr);
	
protected:
	friend class DylibSearch;
	friend class LoadableObject;
	
	void setRequesterRunpaths(const std::vector<const char*>& rpaths) { m_parentRpaths = rpaths; }
	const std::vector<const char*>& runPaths() const { return m_file->rpaths(); }
	
	ProgramVars* getProgramVars();
private:
	void postConstruct();
	
	void loadSegments();
	void unloadSegments();
	void setInitialSegmentProtection();
	void readSymbols();
	void readExports();
	
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
	void writeBind(MachO::Bind* bind, void* addr);
	
	void detectAbsolutePath();
	void* resolveSymbol(const std::string& name);
	
	void jumpToStart();
	
	void setupTLS();
	void teardownTLS();
	
	static int machoProtectionFlagsToMmap(int machoFlags);
	static uintptr_t pageAlign(uintptr_t what);
	static void checkMappingAddr(void* addr);
	static void writeBind(int type, uintptr_t* ptr, uintptr_t newAddr);
	static bool lookupDyldFunction(const char* name, void** addr);
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
	std::vector<const char*> m_parentRpaths;
	void* m_reworkedEHData = nullptr;
	
	std::string m_absolutePath, m_absolutePathDir;
	
	mach_header m_header;
	bool m_bindAllAtLoad;
	
	int m_argc = 0;
	char** m_argv = nullptr;
	char** m_envp = nullptr;
};

#endif
