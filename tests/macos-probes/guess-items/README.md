# macOS black-box probes: GUESS items of open Darling PRs (UNVERIFIED, results pending)

> **Status: NOT RUN ON macOS.** Nothing in this directory has produced a macOS observation yet. Every probe's README says `macOS result: NOT YET OBSERVED`. This is a draft submission asking for verification, not a set of passing tests.

## What this is
28 standalone programs (C or Objective-C, public API only) that print what a real macOS does for the behaviours marked `GUESS` in open draft pull requests by the same author (mostly darlinghq/darling-cocotron and darlinghq/darling-foundation, plus darlinghq/darling #2406). Each `GUESS` is a choice the public documentation does not settle, implemented on inference-tier evidence (AGENTS.md section 3); each affected PR carries a banner saying so. The directory name starts with the PR number (`641-popup-content-bindings` is darling-cocotron #641; `147-...` is darling-foundation #147; `2406-...` is darling #2406). They print `PROBE <id> <input> -> <output>` lines and assert nothing.

The probes were written from the GUESS wording and Apple's public documentation, without reading the PR code, so a result can also show macOS doing something the PR did not expect. Some GUESS items are not observable this way (window-system protocol behaviour, visual choices, Darling internals); `INDEX.md` lists what each probe covers and what it cannot.

## How to run (on a Mac)
In a logged-in GUI session with the command line tools installed: `cd tests/macos-probes/guess-items && sh run-all.sh`. It builds and runs every probe with a time limit and writes `results-batch2-<macOS version>.txt`, with user names, host names and paths stripped. Windows stay at alpha 0 and the app is never activated; opt-in flags are described in `INDEX-B.md` (`--synth` posts events to the probe's own process and may need the Accessibility permission; `--warp` moves the real pointer and is never passed by `run-all.sh`).

## What is asked of reviewers
1. Run the probes on a Mac and attach the results here (or tell us a Mac result already exists).
2. For each affected PR, a maintainer decides keep / change / tag as `QUIRK:` using the observation.
3. Say whether this directory belongs in this repository; closing the PR is a fine answer.

## Provenance
No Apple binary, disassembly, dump or trace was used and no Apple SDK file was copied. Written from Apple's public documentation and the GUESS text of the PRs. Syntax-checked only, against Darling's own SDK tree with stubs for public names it lacks; never built or run.

## Not for merge until
macOS results are attached and a maintainer agrees the directory belongs here.
