# darling-python3

Guest Python 3 support for Darling.

## Upstream URL

https://github.com/python/cpython/tree/v3.13.18

The `cpython` submodule is pinned to the upstream CPython 3.13 security release. Python 3.13 is installed side-by-side with the existing Python 2.7 framework: `/usr/bin/python` defaults to Python 3, and Python 2.7 remains available as `/usr/bin/python2`.

The `openssl` submodule is pinned to OpenSSL 3.5.9. It is built as a private static dependency for the Python 3 `_ssl` and `_hashlib` modules because CPython 3.13 requires OpenSSL 1.1.1 or newer; Python 2 keeps using Darling's existing LibreSSL-backed targets. OpenSSL 4's removed-API list was audited against Darling-owned code: no Darling components use those APIs. CPython 3.13 still calls the deprecated fixed-version SSL/TLS method constructors; upstream CPython is expected to migrate those calls when it is updated for OpenSSL 4, so the private OpenSSL build keeps the default method set.

## License

CPython is distributed under the PSF License Version 2 and related historical Python licenses. See `cpython/LICENSE`. Third-party components included in CPython carry their own notices in the upstream tree.

OpenSSL is distributed under the Apache License 2.0. See `openssl/LICENSE.txt`.

## Provenance

This work follows the public CPython 3.13 build contracts (`configure`, `Makefile.pre.in`, `Modules/makesetup`, `Misc/python-config.in`) and documented macOS framework layout. The TLS change follows CPython's PEP 644 requirement for OpenSSL 1.1.1 or newer. No Apple restricted source, disassembled material, or application-specific behavior was used.

## Updating Source

When updating the source, follow the [`Updating Sources`](https://docs.darlinghq.org/contributing/updating-sources/index.html#updating-sources) documentation and `darling/notes/UPDATE_SOURCE.md`. Keep the version variables in `CMakeLists.txt` in sync with the pinned submodule tag, including framework version, install suffix, bytecode tag, and dependency names.
