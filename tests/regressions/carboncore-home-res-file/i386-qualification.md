# Native Darwin i386 qualification — 2026-11-14

The exact implementation at `b52a309cfb2f4b020aa8e356df107ad92d6a76f3`
was executed as a Mach-O i386 program on the 5700X / Debian WSL2. The earlier
GCC/Clang host matrix was not repeated. No production correction was needed.

## Sources and build

- PR code: `b52a309cfb2f4b020aa8e356df107ad92d6a76f3`.
- Pre-fix reference: `36536a662356001194a7cfad2b42b509003d6bd8`.
- Qualified loader/runtime context: D4rling `4be2603c62a6b0ec0148e60c7dff2e4f8b61c931`.
- Compiler: Clang 19.1.7; target `i386-apple-darwin20`; `-O0 -ggdb`.

All **30 CarbonCore translation units** were rebuilt from an isolated checkout
of the PR, including its upstream `MacMemory.cpp`, `FileManager.cpp`,
`Resources.cpp` and `ResourcesImpl.cpp`. No CarbonCore object from D4rling
master was reused. Source hashes were checked against the PR's Git objects.
Header submodules were materialized locally at the PR's pinned upstream
revisions; those gitlinks match the qualified synchronization build. Generated
configuration headers, cross-linker and real dependency dylibs came from that
existing private build. The full compiler/linker argument vectors and input
hashes are retained in the contributor's local qualification bundle.

Both the declared-dependency framework and the witness linked as real Mach-O
i386 binaries. The native runs used the additional linker option
`-dead_strip_dylibs`: Foundation and libiconv were discarded because no
CarbonCore object references them. All 30 CarbonCore objects remain included.
The resulting dependency set is CoreFoundation, ICU, libc++, libSystem and
libc++abi, using actual Darling libraries. No host Memory Manager, File Manager
or resource adapter was linked. Compiler dependency files were checked for
Linux system headers and test shims; none were present.

**Limit:** the ordinary unpruned dependency graph is still not runtime-qualified
here. Foundation loads CFNetwork, which needs a final i386 Security provider
that is not built in this `gui_stubs` environment. This is a separate local
dependency gap; this note does not claim a successful stock full-system build.

## Observed results

| Check | Observed result |
| --- | --- |
| All 30 CarbonCore sources compiled and linked i386 | PASS |
| Native witness, normal launch | PASS, exit 0, 88 checks |
| Native witness, shellspawn bypass | PASS, exit 0, 88 checks |
| Pre-fix library loaded by native symbol probe | PASS: HomeResFile absent, expected exit 77 |
| Existing qualified Carbon event i386 witness | PASS, exit 0, expected marker |
| Protected source/install inputs | PASS: 5,141 file hashes unchanged |
| Namespace cleanup | PASS: 41 host mounts before/after, no residual darlingserver |
| Unpruned Foundation/CFNetwork/Security runtime graph | BLOCKED: final Security i386 provider unavailable |

The native witness calls the public APIs directly. It covers two resource files
with the same type/ID and different data, both directions of non-current-file
ownership, clearing a prior error, null/unknown/invalid/ordinary memory handles,
release/reload, detach, closing a non-current file, closing the current file,
and successful lookup after an error. Every HomeResFile call captures ResError
and checks that CurResFile is unchanged. The release, detach, close and memory
operations are the actual upstream implementations from this PR's tree.

The candidate's loaded UUID was
`405ad8fe-57c1-3b2c-85e0-6dd8565db4a0` for **HomeResFile, GetHandleSize and
FSPathMakeRef**. This matches the rebuilt candidate and distinguishes it from
the implementation already present in D4rling master.

Artifact SHA-256:

- Candidate with unused dylibs stripped:
  `96ac8130d5b4dfec7fe013cb394b78cdfbed65d42a642b85e9768ad578c58dbb`.
- Equivalent pre-fix reference:
  `db68bce8de1c9680fc5dc8d3853fb1603656318bc7db1ff7c5140613129784d7`.
- Native witness:
  `ebd1c391a422b23fcc8b60e5f1a9c3bee470b6f7f50c87610deeab87aa384e00`.

## Reproduction and isolation

`native-i386.c` is the exact executed witness source. Compile it as a Darwin
i386 C program with the configured Darling SDK/header flags, then link against
the rebuilt CarbonCore and libSystem. Its success marker is
`HOME_RES_I386_PASS checks=88 failures=0`; `native-symbol-probe.c` loads the
framework and returns 77 when the pre-fix HomeResFile export is absent.
Use a fresh private prefix: fixtures use O_EXCL and are intentionally retained.

The actual run reused the synchronization bench's delegated UID/GID mappings,
verified `setgroups=allow` and real setgroups calls, and normalized supplementary
groups with `setpriv --groups=0`. A read-only private installation copy, fresh
short prefixes, private mount/PID/IPC/UTS/network namespaces and a 45-second
timeout were used. No historical prefix, canonical checkout, global install,
master branch or maintainer conversation was modified.

The private bundle contains exact build commands, pinned input manifests,
the adapted delegated runner, raw logs, and a replay command that verifies
inputs before building/running. Public notes omit personal local paths.

Two setup failures are retained and excluded from functional PASS counts:
an initial CFNetwork firstpass selection, and an initial nested file bind that
was invisible through the prefix overlay's lower directory (old UUID observed).
The final runs populated the selected library into the actual private lower
directory before prefix creation and verified its loaded UUID.

Provenance: the public HomeResFile contract and synthetic resource fixtures;
the existing qualified D4rling namespace harness; actual Darling open-source
headers, implementations and libraries. No proprietary SDK, restricted Apple
implementation source or Apple binary inspection was used. AI-assisted
preparation and execution; human contributor handles maintainer discussion.
