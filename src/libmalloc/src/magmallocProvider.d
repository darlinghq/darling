provider magmalloc {
		probe refreshIndex(void *, int, int);
		probe depotRegion(void *, int, void *, int, int);
		probe recircRegion(void *, int, void *, int, int);
		probe allocRegion(void *, int, void *, int);
		probe deallocRegion(void *, void *, int);
		probe madvfreeRegion(void *, void *, void *, int);
		probe pressureRelief(void *, int, int);
		probe mallocErrorBreak();
};

#pragma D attributes Evolving/Evolving/ISA provider magmalloc provider
#pragma D attributes Private/Private/Unknown provider magmalloc module
#pragma D attributes Private/Private/Unknown provider magmalloc function
#pragma D attributes Evolving/Evolving/ISA provider magmalloc name
#pragma D attributes Evolving/Evolving/ISA provider magmalloc args

