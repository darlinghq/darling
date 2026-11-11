// Copyright (C) 2026 Lubos Dolezel
//
// Built as a universal arm64 + arm64e dylib (libdlsymslices.dylib). Each slice
// reports which one it is, so an arm64e program can check that dyld picked
// the arm64e slice.

int dlsym_slice(void)
{
#if __arm64e__
	return 'e';
#else
	return '8';
#endif
}
