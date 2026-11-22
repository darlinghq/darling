# Probe 2406: ASWebAuthenticationSessionRequest and the browser session manager

PR: darlinghq/darling#2406 (AuthenticationServices: implement ASWebAuthenticationSessionRequest and the browser session manager).

**macOS result: NOT YET OBSERVED**

Public API only, written from the public documentation and from the wording of the PR's GUESS items. No private selectors, no private symbols, no Apple binary, header dump or trace was used. The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. It was derived from the GUESS text, not from the PR's code, so it can also show macOS doing something the PR did not expect. Nothing here claims a Mac result; the Darling guest ran nothing.

Window server: no (no window, no browser, no network). Needs macOS 10.15 or later. The probe starts copies of itself (`./probe classes`, `bare`, `delegate`, `manager`) as child processes so a crash in one section is reported as a line (`2406.child ... signal`) and the others still run; run it as `./probe` with no argument.

Most of the PR cannot be tested black-box: a request that carries a URL, UUID, scheme and header fields is only created by the system when another application starts a web authentication session in a browser that registered as a handler. No public initializer exists. The probe therefore observes a bare request made with `-init` (called by selector name, because the header may mark it unavailable), the delegate callbacks on it, and the shared manager.

## The GUESS items and what the probe prints for each

Observable in part:

- "A request made with plain -init has no URL and no UUID (no public initializer is documented)" -> `2406.B.init` (does `-init` return an object or nil or raise), `2406.B.properties` (URL, UUID, callbackURLScheme, shouldUseEphemeralSession, additionalHeaderFields, delegate).
- "A copy is a new object (docs list NSCopying only)" -> `2406.B.copy`, `2406.B.equal`.
- "Decoding fails (returns nil) when the URL or UUID is absent" -> `2406.B.archive`, `2406.B.unarchive` (secure archive and decode of a bare request, which has neither), `2406.B.init-coder` (`initWithCoder:` on an unrelated archive). A populated request with a URL and UUID cannot be made, so the other half (decoding with both present) is not observable.
- "Every completion or cancellation is forwarded to the delegate as made; no once-only rule" -> `2406.C.complete`, `2406.C.complete-twice`, `2406.C.cancel-after-complete`, `2406.C.cancel-twice`, `2406.C.complete-after-cancel`, `2406.C.cancel-nil`, `2406.C.complete-nil`.
  - Second call forwarded: keep. Suppressed: change (once-only). Any call on a bare request being ignored altogether is possible because there is no session; the rows say what macOS does for exactly this object, which may differ from a populated request.
- "Delegate callbacks run synchronously on the calling thread" -> `2406.C.thread` (call from a background queue: logged thread, immediately and after 0.5 s), `2406.C.sync` (main thread), `2406.C.delegate`, `2406.C.delegate-weak` (is the delegate held weakly).
- "sessionHandler is declared non-null as documented but is nil until set" -> `2406.D.handler` (before assignment, after set, read back), `2406.D.handler-weak`, `2406.D.handler-nil`, `2406.D.handler-wrong`, `2406.D.handler-calls`.
- "wasLaunchedByAuthenticationServices is always NO" -> `2406.D.wasLaunched` (in a normal process launched from a shell it is expected NO; this does not show the YES case).
- "`+sharedManager` on a subclass returns the base class instance" -> `2406.D.subclass`; also `2406.D.shared`, `2406.D.init`, `2406.D.copy`, `2406.D.thread`.
- Class shape: `2406.A.class` and `2406.A.protocol` print the superclass, secure-coding support, NSCopying conformance and the required or optional methods of the two protocols.

Not observable (no public construction, or a Darling-only decision), listed here and not forced:

- "The NSSecureCoding archive keys ... Our own keys, chosen to differ from the property names": real archive keys are not public and a populated request cannot be built; reading keys out of an Apple archive was not done.
- "A copy keeps the UUID and the other values but not the delegate": needs a populated request.
- "Apple documents `callbackURLScheme` as deprecated in macOS 14.4 ... headers carry no availability macros" and "The handling protocol's parameters are left unannotated": header annotation choices; compare with the Mac's own header by reading it (a documentation check, not a probe).

## Apple documentation

- https://developer.apple.com/documentation/authenticationservices/aswebauthenticationsessionrequest
- https://developer.apple.com/documentation/authenticationservices/aswebauthenticationsessionrequestdelegate
- https://developer.apple.com/documentation/authenticationservices/aswebauthenticationsessionwebbrowsersessionhandling
- https://developer.apple.com/documentation/authenticationservices/aswebauthenticationsessionwebbrowsersessionmanager

## How to run

Copy this directory to a Mac with the stock Xcode command line tools, then:

```sh
sh build.txt
./probe > results.txt
```

Paste the `PROBE` lines back. Nothing in the output contains user names, host names or paths. Do not commit the probe to Darling: it is an observation tool, not product code (Darling AGENTS.md section 6).

Syntax check done on Linux only: `clang -fsyntax-only -target x86_64-apple-macos11 -isysroot <Darling SDK>`. It parses with no stubs.

## What this probe cannot answer

Anything that needs a real browser-launched session (populated requests, `wasLaunchedByAuthenticationServices` being YES, the handler being called by the system).
