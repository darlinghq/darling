# Probe 0352: NSURL fileReferenceURL, filePathURL and isFileReferenceURL

**macOS result: NOT YET OBSERVED**

Purpose: Prints, as booleans, what the file-reference URL family answers for existing and missing files, directories, symlinks, non-file URLs and a moved file.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

A temporary file, directory and symlink are created. For each: `-fileReferenceURL` (nil or not, is it a reference URL, does its absolute string start with the file-reference prefix), `-filePathURL` of the reference and of a plain URL (same file as the original), `-isFileReferenceURL` of plain and reference URLs, results for missing files and an http URL, a path with a `..` component, and, after the file is moved, whether the old reference URL still reaches it and whether a plain URL of the old path still yields a reference URL. No paths are printed.

## Why it matters, and Darling code affected

The patch answers these three methods by path on a system with no file IDs, so a reference URL would not follow a move. The documented contract says a file reference URL tracks the file's identity. What macOS answers decides which deviations are visible to applications.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- darling-foundation master (read 2026-11-21): src/NSURL.m:850-868 call -isFileReferenceURL and -filePathURL; no definition of -fileReferenceURL, -filePathURL or -isFileReferenceURL was found in src/NSURL.m or src/NSURLInternal.h (declared in include/Foundation/NSURL.h; only _CFURLIsFileReferenceURL is declared at src/NSURLInternal.h:12). A runtime check was not possible.

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
| fileReferenceURL of an existing file is a reference URL (prefix file:///.file/id=) that follows a move | This is macOS. A path-based Darling answer (isFileReferenceURL NO) is a documented deviation; the unrecognized-selector alternative is worse for callers. A deviation needs QUIRK-style documentation but is general. |
| fileReferenceURL of a missing file is nil | Pins the nil case; both Darling variants should match. |
| filePathURL of a non-file URL is nil, of a plain file URL is the same path | Public, general rules independent of file IDs; implement as in the documentation. |
| Old reference URL still reaches the moved file | Cannot be reproduced without a file-ID facility; keep as a known deviation. |

## Public documentation

- https://developer.apple.com/documentation/foundation/nsurl/filereferenceurl()
- https://developer.apple.com/documentation/foundation/nsurl/filepathurl
- https://developer.apple.com/documentation/foundation/nsurl/isfilereferenceurl()

## What this probe cannot answer

Whether Darling should implement file IDs at all (a filesystem design question).

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.
