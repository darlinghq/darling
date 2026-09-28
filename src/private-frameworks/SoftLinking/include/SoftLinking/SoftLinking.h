/*
	SoftLinking.private

	Apple's private framework behind the "softlink" interposition: a library that a
	system framework wants to use if it is present, without binding hard against it.
	Caller loads the library, gets NULL when it is absent, and takes its own
	fallback path.

	Only the entry point the local apps bind is declared. __sl_dlopen does the load
	and returns NULL rather than aborting, because a soft failure is what a soft
	link is for.
*/

#ifndef SOFTLINKING_H
#define SOFTLINKING_H

#ifdef __cplusplus
extern "C" {
#endif

void* __sl_dlopen(const char* path, int mode);

#ifdef __cplusplus
}
#endif

#endif
