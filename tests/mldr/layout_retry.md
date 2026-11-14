# i386 layout retry regression test

From a configured Darling build with `BUILD_TARGET_32BIT` enabled:

```sh
cmake --build build --target mldr32 mldr32_layout_retry_test
ctest --test-dir build -R '^mldr32_layout_retry$' --output-on-failure
```

`layout_retry.c` includes the actual `mldr.c` implementation and builds as an
ELF i386 executable. Linker section collection drops unrelated loader code.
Only host calls needed to control the retry path are replaced; the counter
uses the host libc's real `strtol`, `getenv`, `setenv` and `unsetenv`.

The test covers absent and valid counters (0, 1, 7 and leading-zero 7), the
exhausted budget (8), negative values, empty strings, letters, suffixes,
leading/trailing whitespace, signs, out-of-budget values and numeric overflows.
It injects `setenv` failure with absent and existing counters and verifies that
no exec or cleanup follows. It also covers failed exec cleanup, failed
`readlink`, a free range, mapping errors and a probe at the wrong address.

A child then repeatedly executes the test binary with real `execv` and a
persistent simulated mapping conflict. The parent requires exactly the eight
counter values 1 through 8, followed by termination without another exec.
Pipe polling and the CTest timeout bound the test even if retrying regresses.
All mappings are simulated; no Darling prefix or server is used. This is a
host-side retry-path test, not a Darwin application/runtime qualification.
