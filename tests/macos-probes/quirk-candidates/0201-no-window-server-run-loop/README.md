# Probe 0201: nextEventMatchingMask when the process has no window server connection

**macOS result: NOT YET OBSERVED**

Purpose: Prints, step by step, what NSApplication does for nextEventMatchingMask:untilDate:inMode:dequeue: (return value, wall and CPU time, whether a timer and a main-queue block run) in a session with and without a window server.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

Each step is printed before it runs so an abort still shows how far it got: session dictionary and active display count, `+sharedApplication`, `-setActivationPolicy:`, then one call of `-nextEventMatchingMask:untilDate:inMode:dequeue:` with a one second date while a 0.1 s timer (common modes) and a main-queue block are pending, then a second call with `distantPast`. It prints the return value, wall time, CPU time, timer fires and main-queue runs.

## Why it matters, and Darling code affected

The patch makes the event loop run the Foundation run loop when there is no display. What a real Cocoa process does without a window server (abort, return nil at once, wait until the date, spin) decides whether a Darling guard is needed and what it must do. It needs two runs on one Mac.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSApplication.m:349 (_display = [NSDisplay currentDisplay]), :1572-1612 (-nextEventMatchingMask:... loops until the date passes, messaging _display) [darling-cocotron origin/master cd700cad]
- AppKit/NSDisplay.m:61-70 (+currentDisplay allocates the process display; backend selection at :70-115) and :256 (display -nextEventMatchingMask:) [darling-cocotron origin/master cd700cad]

## How to run

Copy this directory to a Mac with the stock Xcode command line tools (macOS 10.14 or newer), then:

```sh
sh build.txt
./probe
```

`build.txt` holds one command line (run it from this directory). `probe_common.h` is a small output helper shared by all probes. Nothing in the output contains paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code (AGENTS.md section 6).

To get the second run: copy `probe` to a Mac with nobody logged in on the console and run it over ssh (or as a launchd daemon), or use `sudo launchctl asuser` with a UID that has no session. Keep both outputs. run-all.sh only produces the baseline.

## What each outcome would mean for the Darling change

| Observation | Meaning |
|---|---|
| Baseline session: returns nil after about 1000 ms, low CPU, timer fires about 10 times, main-queue block ran | This is the reference for a healthy loop; Darling with no backend should match it. |
| No window server: the process aborts at sharedApplication | macOS does not run an AppKit loop without a window server; Darling must not invent one, and the guard should fail loudly instead. |
| No window server: sharedApplication returns, nextEvent returns nil after the date, timer and queue still run, CPU low | macOS keeps the run loop serviced without a display; the patch direction is right. |
| No window server: nextEvent spins (CPU near wall time) or returns at once | Records a macOS quirk; unlikely, but then the patch must not "fix" it. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nsapplication/nextevent(matching:until:inmode:dequeue:)
- https://developer.apple.com/documentation/coregraphics/cgsessioncopycurrentdictionary()

## What this probe cannot answer

The no-window-server run needs a Mac set up for it (ssh to a Mac with nobody logged in, or a launchd daemon). Not available to the author.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.
