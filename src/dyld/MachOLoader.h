/*
This file is part of Darling.

Copyright (C) 2012 Lubos Dolezel
Copyright (C) 2011 Shinichiro Hamaji

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MACHOLOADER_H
#define MACHOLOADER_H
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <stdint.h>
#include "MachO.h"
#include "arch.h"
#include "ld.h"
#include "UndefinedFunction.h"
#include "Trampoline.h"

class MachOLoader
{
#ifdef __x86_64__
public:
	typedef segment_command_64 Segment;
private:
	static inline const std::vector<Segment*>& getSegments(const MachO& mach) { return mach.segments64(); }
#else
public:
	typedef segment_command Segment;
private:
	static inline const std::vector<Segment*>& getSegments(const MachO& mach) { return mach.segments(); }
#endif

public:
	MachOLoader();
	~MachOLoader();
	
	// Maps module segments into the memory
	void loadSegments(const MachO& mach, intptr* slide, intptr* base);
	
	
	void doRebase(const MachO& mach, intptr slide);
	
	// Puts initializer functions of that module into the list of initializers to be run
	void loadInitFuncs(const MachO& mach, intptr slide);
	
	// Loads libraries this module depends on
	void loadDylibs(const MachO& mach, bool nobinds, bool bindLazy);
	
	// Resolves all external symbols required by this module
	void* doBind(const std::vector<MachO::Bind*>& binds, intptr slide, bool resolveLazy = false);
	
	// Calls mprotect() to switch segment protections to the "initial" value.
	// We initially set the maximum value.
	void doMProtect();
	
	// Creates a list of publicly visible functions in this module
	void loadExports(const MachO& mach, intptr base, Exports* exports);
	
	// Loads a Mach-O file and does all the processing
	void load(const MachO& mach, std::string sourcePath, Exports* exports = 0, bool bindLater = false, bool bindLazy = false);
	
	// Dyld data contains an accessor to internal dyld functionality. This stores the accessor pointer.
	void setupDyldData(const MachO& mach);
	
	// Runs initializer functions that have not been run yet
	void runPendingInitFuncs(int argc, char** argv, char** envp, char** apple);

	// Performs pending binds. Used when loading the main executable and its dependencies.
	void doPendingBinds();
	
	// Starts an application
	void run(MachO& mach, int argc, char** argv, char** envp, bool bindLazy = false);
	
	const std::list<Exports*>& getExports() const { return m_exports; }
	Exports* getMainExecutableExports() const { return m_mainExports; }
	
private:
	// Jumps to the application entry
	void boot(uint64_t entry, int argc, char** argv, char** envp, char** apple);

	// checks sysctl mmap_min_addr
	static void checkMmapMinAddr(intptr addr);
private:
	intptr m_last_addr;
	std::vector<uint64_t> m_init_funcs;
	std::list<Exports*> m_exports;
	Exports* m_mainExports;
	std::vector<std::pair<std::string, uintptr_t> > m_seen_weak_binds;
	UndefMgr* m_pUndefMgr;
	TrampolineMgr* m_pTrampolineMgr;
	
	// Pending calls to mprotect
	struct MProtect
	{
		void* addr;
		size_t len;
		int prot;
	};
	std::vector<MProtect> m_mprotects;

	// Pending libs that require binding
	struct PendingBind
	{
		const MachO* macho;
		const mach_header* header;
		intptr slide;
		bool bindLazy;
	};
	std::vector<PendingBind> m_pendingBinds;
	
	std::vector<std::string> m_rpathContext;
};

#endif
