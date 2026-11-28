# Probe 677: CGAcquireDisplayFadeReservation and CGReleaseDisplayFadeReservation

PR: darlinghq/darling-cocotron#677 (CoreGraphics: CGAcquireDisplayFadeReservation returns a token).

**macOS result: NOT YET OBSERVED**

Public API only, written from the public documentation and from the wording of the PR's GUESS items. No private selectors, no private symbols, no Apple binary, header dump or trace was used. The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. It was derived from the GUESS text, not from the PR's code, so it can also show macOS doing something the PR did not expect. Nothing here claims a Mac result; the Darling guest ran nothing.

Window server: yes (display services in a GUI session; no window is created and nothing is activated). The only `CGDisplayFade` calls go from blend 0 to blend 0 over 0 seconds, so no visible fade is intended. The probe starts copies of itself (`./probe try`, `hold`, `nulltoken`) as child processes; run it as `./probe` with no argument. The probe holds a reservation for up to 5 seconds in places, so run it on a Mac nobody is using for fades.

## The GUESS items and what the probe prints for each

1. "A reservation is per process" -> `677.B.second-same-process` (same process, one held), `677.C.other-process` (a child process acquires while the parent holds one), `677.C.parent-try` (the parent acquires while a child holds one), `677.C.after-child-exit`.
   - Child acquire succeeds (0) while the parent holds one: reservations are per process, keep. Child refused with `kCGErrorNoneAvailable` (compare the printed number with the value in CGError.h): the hold is system-wide, change.
2. "The valid interval: `(0, kCGMaxDisplayReservationInterval]` (function page) or 1 to the maximum (CGDisplayReservationInterval page). An interval outside it and a NULL token pointer return `kCGErrorIllegalArgument`" -> `677.A.interval` (0, -1, -0, 1e-6, 0.0001, 0.5, 0.999, 1, 2, 14.999, the maximum (15), the maximum plus 0.001 and plus 1, 100, +inf, -inf, NaN, 1e30; each acquired then released, with the result code and the token), `677.D.null-token` (child process, NULL token pointer).
   - Which of 0 / 0.5 / 0.999 succeed decides between the two documented ranges. The code printed for the rejected intervals (and whether the token is left untouched: `(untouched)`) is the answer to "no documented error". A crash of the NULL-pointer child shows as a `677.child ... signal` line: then the PR returning an error is a deviation worth a comment.
3. "A token that was never issued, given to Release, returns `kCGErrorIllegalArgument`. `CGDisplayFade` does not look at the token at all" -> `677.D.release-unissued` (0, 1, 2, 12345, 0xFFFFFFFF with nothing held), `677.B.release-first-again` (double release), `677.B.stale-release` and `677.B.new-still-held` (an old token must not end a newer reservation), `677.E.expiry` (release of an expired token), `677.F.fade` (CGDisplayFade with a valid, a released, zero, never-issued and an expired token).
   - Release of a never-issued token returning 0 on macOS: change the PR (it should not be an error). Returning an error code: compare the number. CGDisplayFade returning an error for released, expired or never-issued tokens: the PR's "CGDisplayFade is unchanged" needs a token check.
4. "Release is documented to unfade the display if needed. Nothing is faded here, so Release only ends the reservation" -> not observable here as a visual; the probe only prints result codes. (A fade to a non-zero blend is deliberately not run.)
5. "`kCGDisplayFadeReservationInvalidToken` ... without a value ... tokens issued here are never 0" -> the header lines at the top (`kCGDisplayFadeReservationInvalidToken` value if the SDK defines the macro, `sizeof` of the token and interval types), and the `token=` values in every acquire line (never 0? increasing? the same value again after release: `677.B.new-token-differs`).
   - Real tokens that can be 0 or repeat: the PR's "never 0" and "new reservation, new token" are stronger than macOS needs; harmless but worth a note.
6. Expiry ("the token becomes invalid at the end of the interval") -> `677.E.expiry`: acquire 1.0 s, acquire again at 0.5 s (refused), at 1.8 s (accepted), release of the expired token must not end the newer reservation.

## Apple documentation

- https://developer.apple.com/documentation/coregraphics/cgacquiredisplayfadereservation(_:_:)
- https://developer.apple.com/documentation/coregraphics/cgreleasedisplayfadereservation(_:)
- https://developer.apple.com/documentation/coregraphics/cgdisplayfadereservationtoken
- https://developer.apple.com/documentation/coregraphics/cgdisplayfade(_:_:_:_:_:_:_:_:)

## How to run

Copy this directory to a Mac with the stock Xcode command line tools, then:

```sh
sh build.txt
./probe > results.txt
```

Run it in a logged-in GUI session (a terminal window), not over ssh with nobody logged in. Paste the `PROBE` lines back. Nothing in the output contains user names, host names or paths. Do not commit the probe to Darling: it is an observation tool, not product code (Darling AGENTS.md section 6).

Syntax check done on Linux only: `clang -fsyntax-only -target x86_64-apple-macos11 -isysroot <Darling SDK>` with a scratch header (-include) declaring `CGAcquireDisplayFadeReservation` and `CGReleaseDisplayFadeReservation` with their public signatures, because Darling's SDK header lacks them (this is the gap the PR fills). On the Mac the real header supplies them; the probe needs no stub there.

## What this probe cannot answer

Whether Release restores a faded display (a visual, hardware-dependent effect); anything about what a second application sees besides acquire results.
