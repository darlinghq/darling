/*
 * version_info.c (ARM)
 *
 * Provides the `___Libm_version` symbol that Apple's tooling stamps into
 * every shipped libm. Apple's Xcode project sets LIBM_VERSION_STRING per
 * build (`RC_ProjectNameAndSourceVersion`); we hard-code a Darling-flavored
 * string here since the CMake build doesn't carry that infra.
 *
 * The `@(#)` prefix is the SCCS what-string marker, matching Apple's
 * convention — `what(1)` will still recognize the binary.
 */

#if defined(__aarch64__) || defined(__arm64__)

#ifndef LIBM_VERSION_STRING
#define LIBM_VERSION_STRING "Libm-darling-arm64"
#endif

const char __Libm_version[] =
    "@(#) " LIBM_VERSION_STRING " " __DATE__ " " __TIME__;

#endif /* __aarch64__ || __arm64__ */
