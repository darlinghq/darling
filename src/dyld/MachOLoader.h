#ifndef MACHOLOADER_H
#define MACHOLOADER_H

class MachOLoader
{
#ifdef __x86_64__
	typedef uint64_t intptr;
	typedef segment_command_64 Segment;

	static inline const vector<Segment*>& getSegments(const MachO& mach) {
		return mach.segments64();
	}
#else
	typedef uint32_t intptr;
	typedef segment_command Segment;

	static inline const vector<Segment*>& getSegments(const MachO& mach) {
		return mach.segments();
	}
#endif

public:
	MachOLoader();
	void loadSegments(const MachO& mach, intptr* slide, intptr* base);
	void doRebase(const MachO& mach, intptr slide);
	void loadInitFuncs(const MachO& mach, intptr slide);
	void loadDylibs(const MachO& mach);
	void doBind(const MachO& mach, intptr slide);
	void loadExports(const MachO& mach, intptr base, Exports* exports);
	void loadSymbols(const MachO& mach, intptr slide, intptr base);
	void load(const MachO& mach, Exports* exports = NULL);
	void setupDyldData(const MachO& mach);
	void runInitFuncs(int argc, char** argv, char** envp, char** apple);
	void run(MachO& mach, int argc, char** argv, char** envp);
private:
	void boot(uint64_t entry, int argc, char** argv, char** envp);
	void pushTrampolineCode(unsigned int c);
	void pushTrampolineCode64(unsigned long long c);
	void pushTrampolineCode32(unsigned int c);
};

#endif
