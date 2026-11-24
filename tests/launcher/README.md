<!-- Copyright (C) 2026 Lubos Dolezel -->

# Launcher host tests

Run the prefix classification and named profile tests without root or a Darling
container:

```sh
cc -std=gnu11 -Wall -Wextra -Werror tests/launcher/prefix-state.c -o /tmp/prefix-state-test
/tmp/prefix-state-test
cc -std=gnu11 -Wall -Wextra -Werror tests/launcher/profile-prefix.c -o /tmp/profile-prefix-test
/tmp/profile-prefix-test
```

The missing-init harness compiles the launcher startup branch with a mocked
process scan and verifies that a live server or inspection error causes no
prefix mutations:

```sh
python3 tests/launcher/test-missing-init.py src/startup/darling.c --run
```

The stale pid file harness compiles the launcher's `findInitProcess()` and checks
that a darlingserver that exited but was not reaped (a zombie) counts as no
server and its pid file is removed:

```sh
python3 tests/launcher/stale-init-pid.py src/startup
```

## Modes

```sh
cc -std=gnu11 -Wall -Wextra -D_GNU_SOURCE -Isrc/startup tests/launcher/mode.c src/startup/launcher-mode.c src/startup/launcher-server.c -o /tmp/mode-test
/tmp/mode-test
```

covers how the launcher tells it is privileged (effective uid 0, a secure exec or any capability; privileged
without uid 0 is refused), the mode selection (a privileged launcher ignores `DARLING_NONROOT`/`DARLING_ROOTLESS`), the install
prefix taken from the executable's path in the unprivileged modes only, the user name rules and the process
tree walk used by the non-root shutdown, and the namespace test that makes an unprivileged launcher refuse a container started in another mode.

Manual checks in an installed Darling (not part of the automated tests):

- `DARLING_ROOTLESS=1 darling shell true` with a non-setuid `darling`, then `darling shutdown`.
- `DARLING_NONROOT=1 darling shell true` likewise; the prefix's `/proc` is a link to `/Volumes/SystemRoot/proc`.
- With the setuid `darling`, `DARLING_NONROOT=1 darling shell true` still runs in the root mode.

## Default locale

```sh
cc -std=gnu11 -Wall -Wextra -D_GNU_SOURCE -Isrc/startup tests/launcher/locale.c src/startup/launcher-locale.c -o /tmp/locale-test
/tmp/locale-test
```

covers the mapping from `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LC_TIME` and `LANG` to the `AppleLocale` and `AppleLanguages` a new prefix gets in its
`/Library/Preferences/.GlobalPreferences.plist`, and that existing values there are never changed.

## Host paths in the guest

```sh
cc -std=gnu11 -Wall -Wextra -D_GNU_SOURCE -Isrc/startup -Ibuild/src/include tests/launcher/path.c src/startup/launcher-path.c -o build/launcher-path-test
build/launcher-path-test
```

covers how `darling open <file>...` and `darling <program-path>` name a host file in the guest: its real path (relative
paths, `.`/`..` and symbolic links resolved, names kept byte for byte) under `/Volumes/SystemRoot`, `/` as
`/Volumes/SystemRoot` itself, and `ENOENT` for a missing file or a dangling link.

## Running a Mach-O file by name (binfmt_misc)

```sh
for opt in -O0 -O2; do
	cc -std=gnu11 $opt -Wall -Wextra -D_GNU_SOURCE -Isrc/startup tests/launcher/binfmt.c src/startup/launcher-binfmt.c -o /tmp/binfmt-test
	/tmp/binfmt-test
done
tests/launcher/binfmt-kernel.sh
```

The first covers how the launcher turns the arguments the kernel builds for its binfmt_misc entry
(`<interpreter> <path> <arguments...>`, descriptor in `AT_EXECFD`) into the `darling exec` form. The second
registers an entry with the same flags as `lib/binfmt.d/darling.conf` in a binfmt_misc mount of a private user
namespace (Linux 6.7 or later, no root, nothing registered on the host) and checks that the kernel passes what the
launcher assumes; it exits with 77 when the system does not allow that. The unit test is built at -O0 and -O2
because an optimizing build reuses stack space that a buffer of the launcher's `main` had left.

Manual check on a real host (not part of the automated tests; registers an entry system-wide and needs root):

```sh
# an installed Darling: lib/binfmt.d/darling.conf names <prefix>/bin/darling, setuid root, as the interpreter
sudo systemctl restart systemd-binfmt     # or: sudo systemd-binfmt /usr/lib/binfmt.d/darling.conf
cat /proc/sys/fs/binfmt_misc/Mach-O-64-bit   # interpreter .../bin/darling, flags: O
./some_macos_binary arg1 "arg 2"           # the same as: darling exec ./some_macos_binary arg1 "arg 2"
# remove an entry again: echo -1 | sudo tee /proc/sys/fs/binfmt_misc/Mach-O-64-bit
```

The fat Mach-O entry matches the magic `0xCAFEBABE`, which Java class files share: an executable `.class` file now
starts the launcher and fails there.
