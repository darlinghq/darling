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
