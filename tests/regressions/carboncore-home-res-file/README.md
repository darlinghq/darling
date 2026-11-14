# HomeResFile host regression

```sh
bash tests/regressions/carboncore-home-res-file/run.sh g++
bash tests/regressions/carboncore-home-res-file/run.sh clang++
DARLING_HOME_RES_FILE_OPT=-O2 bash tests/regressions/carboncore-home-res-file/run.sh g++
DARLING_HOME_RES_FILE_OPT=-O2 bash tests/regressions/carboncore-home-res-file/run.sh clang++
python3 -B tests/regressions/carboncore-home-res-file/check_mutations.py g++
python3 -B tests/regressions/carboncore-home-res-file/check_mutations.py clang++
```

Each run compiles the real Resources.cpp, ResourcesImpl.cpp and public
CarbonCore/Resources.h with ASan, UBSan and leak detection. Synthetic files
contain resources with identical type/ID and different data in separate files.

The contract comes from Apple's public
[HomeResFile reference](https://developer.apple.com/library/archive/documentation/mac/MoreToolbox/MoreToolbox-48.html):
return the owning file reference and noErr, or -1 and resNotFound for a handle
not associated with a resource. Lookup must not select a new current file.

The suite checks both directions across two files, resetting a previous error,
null and unknown handles, an ordinary memory handle, an empty master pointer,
release, detach and file close. It captures ResError before checking CurResFile.
A failed eager handle allocation also checks that a null key in the existing
owner map never makes NULL valid.

The shims replace File Manager path resolution, Memory Manager allocation and
unused CoreFoundation/xattr interfaces for a Linux host build. The real reader,
ownership tables, lookup and release/detach/close paths are used. No installed
framework, Darling server or historical prefix is used.

These tests do not validate the Darwin File Manager/Memory Manager ABI,
resource-fork xattrs, System/ROM resource setup, persistent writing, or a complete
CarbonCore framework/application. Stale handle values are queried before any
new handle allocation could reuse their addresses; this does not promise to
distinguish arbitrary stale addresses after reuse.

The production change adds only HomeResFile. Existing allocation, lifetime,
current-file selection and writer behavior are unchanged. Unrelated lifecycle
edge cases remain outside this suite.

Native Darwin i386 execution and its configuration limit are documented in
[i386-qualification.md](i386-qualification.md). The native witness sources are
separate from the host-adapter suite above.
