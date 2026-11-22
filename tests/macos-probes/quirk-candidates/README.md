# macOS black-box probes: quirk candidates (UNVERIFIED, results pending)

> **Status: NOT RUN ON macOS.** Nothing in this directory has produced a macOS observation yet. Every probe's README says `macOS result: NOT YET OBSERVED`. This is a draft submission asking for verification, not a set of passing tests.

## What this is
Small standalone programs (C or Objective-C, public API only) that print what a real macOS does for 23 questions about undocumented behaviour, in the form `PROBE <id> <input> -> <output>` (AGENTS.md sections 3 and 7: black-box observation of the public API is the acceptable evidence tier for a quirk). They were written from Apple's public documentation to answer "what does macOS do here?" for behaviour that Darling's maintainers may want to match, such as control metrics, bindings edge cases, `NSURL` file-reference URLs or `NSIntersectionRect` with unbounded edges. The directory names start with the number of the siliconforest patch that raised the question, or `item-<letter>` for questions raised while reviewing other changes.

They are NOT part of the guest test suite (`tests/src`): they print observations and assert nothing, and they are meant to be run on macOS, then optionally on Darling for comparison.

## How to run (on a Mac)
In a logged-in GUI session with the command line tools installed: `cd tests/macos-probes/quirk-candidates && sh run-all.sh`. It builds each probe with its `build.txt`, runs it with a time limit and writes one `results-<macOS version>.txt`; user names, host names and paths are stripped. `INDEX.md` lists the probes and the extra steps some need (0201 needs a second run with no window server reachable).

## What is asked of reviewers
1. Run the probes on a Mac (or tell us where a reviewer-owned Mac result already exists) and attach the results to this PR.
2. Say whether a directory like this belongs in this repository at all, or somewhere else. This was written for a specific verification task; closing the PR is a fine answer.

## Provenance
No Apple binary, disassembly, dump or trace was used, and no Apple SDK file was copied. The probes were written from Apple's public documentation. The questions come from reading patches in the siliconforest project (https://github.com/chriskearney/siliconforest, commit 21703928bf) to identify behaviour the documentation does not settle; no code or comments from those patches were copied. Syntax-checked only, against Darling's own SDK tree, with stubs for public names it lacks; never built or run.

## Not for merge until
macOS results are attached and a maintainer agrees the directory belongs here.
