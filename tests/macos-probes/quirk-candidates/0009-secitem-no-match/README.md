# Probe 0009: SecItemCopyMatching: status and result pointer when nothing matches or the query is malformed

**macOS result: NOT YET OBSERVED**

Purpose: Prints the status and what is left in the result pointer for read-only queries that match nothing, for a NULL result pointer, for client-certificate enumeration and for malformed queries.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

Queries are read-only (no add or delete): generic password with a unique service (return attributes, return data with match limit all, no return type, NULL result pointer), identity enumeration with match limit all, certificate and key with a unique label, internet password, a data-protection-keychain query (which needs an entitlement and shows what an unsigned binary gets), an empty query, a query without a class and an invalid class value. The result pointer is preset to a sentinel so "left untouched" is visible.

## Why it matters, and Darling code affected

Security rule: a quirk in security-sensitive code is never implemented without explicit maintainer approval (AGENTS.md section 3). The patch makes a not-found search return errSecItemNotFound and sets the result to NULL on every path. The documentation names errSecItemNotFound for "not found", but what the result pointer holds is not stated; this probe produces the evidence; the approval stays with a maintainer.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- darling-security master (read 2026-11-21): OSX/sec/Security/SecItem.c:1771-1809 (SecItemCopyMatching: *result is not initialised; status is returned as computed, so "no daemon" can come back as success with nothing written) [master as read]

## How to run

Copy this directory to a Mac with the stock Xcode command line tools (macOS 10.14 or newer), then:

```sh
sh build.txt
./probe
```

`build.txt` holds one command line (run it from this directory). `probe_common.h` is a small output helper shared by all probes. Nothing in the output contains paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code (AGENTS.md section 6).

## What each outcome would mean for the Darling change

| Observation | Meaning |
|---|---|
| Every no-match query returns errSecItemNotFound and result NULL | Documented status plus a general "result defined on every path" rule; Darling should match, subject to maintainer approval. |
| Result left untouched on some paths (for example a malformed query) | macOS does not define the result on error; then only the errSecItemNotFound case should be ported. |
| Data-protection query returns errSecMissingEntitlement | An unsigned or ad-hoc signed process gets that status on macOS; useful for a guest with no keychain daemon. |
| NULL result pointer returns errSecItemNotFound or errSecSuccess | Existence checks depend on it. |

## Public documentation

- https://developer.apple.com/documentation/security/secitemcopymatching(_:_:)
- https://developer.apple.com/documentation/security/errsecitemnotfound
- https://developer.apple.com/documentation/security/security-framework-result-codes

## What this probe cannot answer

The approval itself: this is security-sensitive code, so a maintainer must approve any change even with full black-box evidence. A keychain with real items on the test Mac is not queried (unique labels only).

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.
