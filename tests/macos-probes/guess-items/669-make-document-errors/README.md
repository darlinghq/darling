# Probe 669: makeDocumentWithContentsOfURL:ofType:error: error reporting through every open path

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #669. Window server: yes (no window is made by the probe; NSApplication is subclassed to observe error presentation; no activation). Arguments: none.

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: Commit 1: the code of the generic error (NSFileReadUnknownError) when initialisation fails without an error.

- Printed: Types ProbeInitNoErr and ProbeReadNoErr (return nil/NO without setting an error): `669.make` lines show the error the controller returns.
- Outcomes: NSCocoaErrorDomain/256: keep. Another code or nil: change.

### GUESS: Commit 2: that the controller presents the error at all.

- Printed: For make..., open...completionHandler:, open...error:, reopen... : the `PRESENT` entries come from NSApplication subclass overrides of presentError: and presentError:modalForWindow:..., and `MODAL-WINDOW(...)` if the controller starts a modal alert itself (aborted after 1 s). `visible windows=` lists windows after the call.
- Outcomes: No PRESENT/MODAL-WINDOW for a direct make... call and for the handler-based calls: the controller reports only through the return/handler (keep: do not present in make...). A PRESENT or modal window for some open path: change (present there; the lines say which path).

### GUESS: Commit 2: that a user-cancelled error is not presented.

- Printed: Types ProbeReadCancel and ProbeInitCancel return NSUserCancelledError (NSCocoaErrorDomain/3072); compare their PRESENT/MODAL lines with ProbeReadErr and ProbeInitErr on each path.
- Outcomes: Cancelled error: no presentation while the other errors are presented on the same path: keep. Both presented, or neither: change.

### GUESS: (related) which initialiser the controller calls and the error passed through.

- Printed: The `calls=[...]` lists initWithContentsOfURL:ofType:error:, initForURL:..., initWithType:error:, readFromURL:ofType:error: in order, and the returned error (is ProbeDomain/7 or /8 passed through unchanged?).
- Outcomes: Informational; decides whether the PR's initialiser choice matches.

## How to run

```sh
sh build.txt
./probe
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

The interactive Open panel and application:openFile: paths are not exercised (they need a person or LaunchServices). If the controller presents errors through a path other than NSApp presentError: (for example an NSAlert run modally), the watchdog aborts it after 1 s and reports MODAL-WINDOW.

## Public documentation

- https://developer.apple.com/documentation/appkit/nsdocumentcontroller/makedocument(withcontentsof:oftype:)
- https://developer.apple.com/documentation/appkit/nsdocumentcontroller/opendocument(withcontentsof:display:completionhandler:)

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.
