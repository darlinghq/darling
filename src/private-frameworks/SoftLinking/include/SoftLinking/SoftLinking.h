/*
	SoftLinking.private

	Apple's private framework behind the "softlink" interposition: a library that a
	system framework wants to use if it is present, without binding hard against it.
	The caller loads the library, gets NULL when it is absent, and takes its own
	fallback path.

	Only the two entry points the local apps bind are declared. sl_dlopen does the
	load and returns NULL rather than aborting, because a soft failure is what a soft
	link is for. TSUSoftLinkingGetFrameworkFunction resolves a symbol out of a
	softlinked framework, and reports NULL when there is no such framework.
*/

#ifndef SOFTLINKING_H
#define SOFTLINKING_H

#ifdef __cplusplus
extern "C" {
#endif

void* _sl_dlopen(const char* path, int mode);
void* TSUSoftLinkingGetFrameworkFunction(const char* framework, const char* function);

#ifdef __cplusplus
}
#endif

#endif
