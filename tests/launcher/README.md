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

## Modes

```sh
cc -std=gnu11 -Wall -Wextra -D_GNU_SOURCE -Isrc/startup tests/launcher/mode.c src/startup/launcher-mode.c src/startup/launcher-server.c -o /tmp/mode-test
/tmp/mode-test
```

covers how the launcher tells it is privileged (effective uid 0, a secure exec or any capability; privileged
without uid 0 is refused), the mode selection (a privileged launcher ignores `DARLING_NONROOT`/`DARLING_ROOTLESS`), the install
prefix taken from the executable's path in the unprivileged modes only, the user name rules and the process
tree walk used by the non-root shutdown.

Manual checks in an installed Darling (not part of the automated tests):

- `DARLING_ROOTLESS=1 darling shell true` with a non-setuid `darling`, then `darling shutdown`.
- `DARLING_NONROOT=1 darling shell true` likewise; the prefix's `/proc` is a link to `/Volumes/SystemRoot/proc`.
- With the setuid `darling`, `DARLING_NONROOT=1 darling shell true` still runs in the root mode.
