# ARM64 lazy-wrapper vector arguments

On a native AArch64 Linux host with Ruby, clang and clang++ installed, run:

    ruby tests/wrapgen-arm64-vectors.rb .

The test builds a small ELF library and the actual wrapper generator, generates
its wrapper, and assembles it for Darwin ARM64. It then extracts the generated
common trampoline, translates only Mach-O visibility directives to ELF, and
executes it with eight 128-bit vector arguments and one stack-passed vector.
The resolver deliberately destroys v0-v7. Both the slow and cached paths must
preserve all lanes, and resolution must occur exactly once.

A negative control narrows the save/restore instructions back to d registers
and requires the runtime test to fail. Temporary files are removed on exit.

This is a native instruction/argument-preservation regression, not an end-to-end
Darling loader test or validation of every Darwin/Linux ABI difference. Rebuild
wrapgen and regenerate existing wrappers to apply the source fix to a build.
