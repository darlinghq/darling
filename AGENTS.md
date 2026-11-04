# AGENTS.md

> **Note to human contributors and reviewers.** This document tells an AI
> agent how to behave *if* you use one. It is not permission to submit code
> you do not understand. You are responsible for every line in a PR you open,
> AI-assisted or not: you must be able to explain what it does, why it is
> correct, and why it follows the rules below, in your own words. "The agent
> wrote it and followed AGENTS.md" is not an acceptable answer to a reviewer's
> question, and a PR whose author cannot explain their own change may be
> closed regardless of how it was produced.
>
> When a maintainer asks a question on a PR or issue, we expect a reply from
> you, the human contributor, not from your AI assistant pasted verbatim, and
> not from the AI replying on your behalf. Maintainers are volunteering their
> time to review your contribution; having that time spent talking to a bot
> instead of you is disrespectful and such comments may be disregarded.

This project is a clean-room-style reimplementation of macOS/Apple platform
functionality. The goal is **API compatibility with the publicly documented
contracts**, while avoiding legal risk and low-quality, application-specific
hacks.

These rules apply to every agent and contributor. When a rule conflicts with
making a task "work", the rule wins. If you cannot complete a task without
breaking a rule, stop and say so (see Section 10).

---

## 1. Implement the documented contract, not a particular application

- Target the behavior described in public API documentation, public headers,
  standards, and RFCs. These are the source of truth.
- Never add special cases, bundle-ID checks, app-name checks, version sniffing,
  or "if this app calls it, return X" logic to make one application work.
- If an application fails, find the **general** documented behavior we got
  wrong or haven't implemented, and fix that.
- Where documentation is ambiguous or silent, first prefer the most
  conservative, spec-consistent behavior. If real-world compatibility requires
  more than that, follow Section 3 (Undocumented behavior and quirks). Never
  guess silently.
- Match documented error codes, return values, threading/reentrancy guarantees,
  and nullability contracts, not just the happy path.

## 2. Follow the macOS layering and dependency hierarchy

Apple's platform is a stack: higher-level frameworks are built on lower-level
frameworks and libraries. Applications are written against that stack and
implicitly rely on its structure. Our implementation must have the same shape.

- **Mirror the layering.** If framework A sits on top of library B on macOS,
  our A must be implemented on top of our B, through B's interfaces. Do not
  re-implement B's functionality privately inside A.
- **Fix things at the right layer.** A bug or missing behavior belongs in the
  lowest layer where it originates. Do not patch it in a higher layer to hide
  a lower-layer defect, and do not duplicate logic across layers.
- **Preserve the compatibility assumption between layers.** Applications that
  use framework A assume library B is compatible with what A needs, and
  applications may also call B directly alongside A. Changes to B must keep
  both usage patterns working. Behavior, object identity, ownership, error
  propagation, and state shared between layers must be consistent whether an
  application enters through A or through B.
- **No dependency inversions.** Lower layers must not depend on higher layers,
  and no cyclic dependencies may be introduced. Do not make a layer reach
  around the one directly beneath it unless that is how macOS does it.
- **Preserve interposition and substitution points.** Where applications or
  system components can observe or override a lower-level call (callbacks,
  delegates, notifications, dynamic dispatch), the higher layer must go
  through that same path rather than a private shortcut.
- **Establish the hierarchy from evidence.** Use public documentation,
  published architecture overviews, public headers (what imports or wraps
  what), linkage visible via public tooling (e.g. which libraries a public
  binary declares as dependencies), and backtraces provided under Section 7.
  If the correct layer for something is unclear, ask (Section 10).
- **Place quirks at the layer where they originate**, not where they were first
  noticed.

## 3. Undocumented behavior and quirks

Public documentation is incomplete, and real applications sometimes depend on
behavior the original implementation exhibits but never documented. We support
such quirks deliberately and carefully, as a last resort, never casually and
never as a shortcut.

### When a quirk may be implemented

All of the following must hold:

1. **Documented behavior is insufficient.** The public contract, standards, and
   conservative interpretation do not produce correct compatibility.
2. **There is acceptable evidence** that the original behaves this way (see
   "Acceptable evidence").
3. **It is general.** The quirk is a property of the API under certain inputs or
   states, not a reaction to a specific application. Test: would any
   application making the same call get the same result? If the behavior only
   triggers for a given bundle ID, app name, version, or call pattern unique
   to one app, it is a hack and is forbidden (Section 1).
4. **It is the minimal behavior needed.** Replicate the observable effect, not
   guessed internals. Do not speculate about why the original does it.

### Acceptable evidence (strongest first)

- Public, license-clean documentation from other sources: Apple technical
  notes, release notes, WWDC material, published books, reputable third-party
  write-ups of observed behavior (paraphrased, never copied).
- Black-box observation of the original through its public API, captured as a
  reproducible behavioral test (inputs, outputs, error codes, OS version).
- **Backtraces from real macOS** supplied by a human (see Section 7). These are
  evidence of *what called what, in what order, in what context*, which makes
  them especially valuable for layering (Section 2), call ordering, threading
  context, and error propagation. They show behavior, not implementation.
- Multiple independent, credible reports of the same observed behavior from
  public bug trackers, forums, or other open-source compatibility projects,
  cited by link. Summarize them; do not copy code from them.
- A strong inference (e.g. a documented sibling API behaves this way, and
  applications widely rely on it), clearly labeled as inference with the
  reasoning stated. Inference alone is the weakest tier and requires
  maintainer approval.

### Never acceptable evidence

- Disassembly, decompilation, or instruction-level inspection of Apple
  binaries, and leaked or confidential Apple material. (Backtraces supplied
  under Section 7 are not in this category; disassembly listings that
  accompany them are.)
- Apple's corecrypto source (see Section 5), or any restricted source.
- Verbatim code from third-party projects with an undeclared or incompatible
  license. Other compatibility projects may inform *what* the quirk is, not
  *how* we write it.

### How to implement a quirk

- **Isolate it.** Put the quirk behind a clearly named helper or branch so it
  can be found, reviewed, and removed. Do not scatter it through mainline
  logic.
- **Mark it in a code comment.** Every quirk carries a comment starting with
  the tag `QUIRK:` placed directly at the quirk. The comment must be
  self-contained and state:
  - the observed behavior, in neutral terms,
  - the evidence and its tier (and a link if public),
  - affected OS versions, if known,
  - the test that pins it.

  Example:
  ```c
  // QUIRK: returns EINVAL instead of success for zero-length input.
  // Not documented. Evidence: black-box observation on macOS 14.x
  // (tier 2); same behavior reported in <public URL>.
  // Test: test_quirk_zero_length_input_returns_einval
  ```
- **Test it.** Add a test that fails without the quirk. The test name or a
  comment must make clear it covers a quirk and reference the same behavior
  the code comment describes.
- **Scope by OS version only if the evidence shows version dependence.** Never
  scope by application.
- **Do not weaken documented behavior** for cases the documentation does cover.
  A quirk applies only to the gap.
- **Security-sensitive quirks** (crypto, authentication, sandboxing, signature
  or certificate validation, memory safety) are never implemented without
  explicit maintainer approval. Compatibility must not silently weaken
  security.

### Distinguishing quirks from hacks

| Quirk (allowed, with evidence) | Hack (forbidden) |
|---|---|
| API returns a specific error code for an undocumented edge-case input, for all callers | Return a canned value when the caller is `com.vendor.App` |
| Preserves an observed ordering or default value that many apps rely on | Skip a failing call because one app crashes on it |
| Marked with a `QUIRK:` comment, evidence stated, and pinned by a test | Explained only by "makes app X work" |

If the real-world trigger was one application, use that only to *discover* the
behavior. Then reproduce the general behavior with a standalone test that does
not involve that application.

## 4. No verbatim copying; use sources for understanding only

- Do not copy code, comments, distinctive structure, or test fixtures verbatim
  (or near-verbatim) from any external source unless its license is explicitly
  declared and permissive **and** compatible with this project's license.
- "Found on the internet / in a repo / in a gist / in a Stack Overflow answer"
  is not a license. No declared license means no permission.
- Third-party code you discover may inform your understanding of *what* needs
  to be done. Close it, then write the implementation independently, in your
  own structure and style.
- If permissively licensed code is genuinely reused, it must be recorded
  (source URL, commit, license, copyright notice) in the PR and in the
  project's third-party notices, and required attribution must be preserved.
- Do not reproduce Apple's documentation prose verbatim in comments or docs.
  Paraphrase and link to the public documentation instead.

## 5. CoreCrypto (and alike): absolute prohibition (highest priority)

- Apple's open-source CoreCrypto (corecrypto) is under a license that is
  incompatible with this project. **No code from it may ever enter this
  codebase under any circumstances**: not copied, not translated to another
  language, not "lightly refactored", not reproduced from memory, not used as
  a line-by-line reference while writing.
- **This extends to any other Apple source we cannot freely use, however it
  became available.** Apple occasionally publishes source for a component
  under terms that make the *source available to read* without it being
  *free/open-source software* we can actually use here (e.g.
  viewing/research-only terms, no-redistribution or no-derivative clauses,
  patent carve-outs, or anything short of a license compatible with this
  project's license). The same is true, with even less ambiguity, of Apple
  source that was *leaked* rather than intentionally released — i.e. it
  became "available" through a breach, an unauthorized dump, or similar,
  with no license from Apple at all. It makes no difference whether such
  source arrived via an official-looking release or a leak, how it was found
  (a public repo, a forum, a pastebin, a torrent, etc.), or whether it
  carries Apple's name or copyright notice: if it is not under a license
  compatible with this project's license, it is restricted material, full
  stop, and every rule in this section that applies to corecrypto applies
  identically to it, whether or not it is named here: no copying,
  translating, "light refactoring", reproduction from memory, or use as a
  line-by-line reference; no pasting it into prompts, tool calls, scratch
  files, or PR text; implementation only from the public API contract,
  published standards, and public test vectors; and the same mandatory human
  review step below before any PR touching a reimplementation of it can be
  merged. If you are unsure whether source you have encountered falls into
  this category, treat it as if it does and ask a maintainer (Section 10)
  before using it for anything beyond understanding the public contract.
- Do not open, quote, summarize, or paste corecrypto source into prompts, tool
  calls, scratch files, or PR text.
- Implement CoreCrypto-compatible APIs from: the public API contract,
  published standards (FIPS, NIST SP, RFCs), and public test vectors (e.g.
  NIST CAVP, Wycheproof, RFC appendices).
- Prefer well-known, permissively licensed, vetted crypto implementations as
  backends over writing primitives from scratch. Do not invent cryptography.
- Treat anything resembling corecrypto's internal structure, naming of
  internals, or algorithms-as-written as a red flag. If you notice that your
  output might be derived from it, discard it and start over from the
  standards, then tell the maintainers.
- Backtraces that pass through corecrypto may be used only to understand which
  public API calls into it and in what order. They must never be used to infer
  or reconstruct anything about corecrypto's internals.
- Quirks in crypto code require explicit maintainer approval (Section 3).
- **Mandatory human review step.** Any PR touching the CoreCrypto
  reimplementation, or the reimplementation of any other component covered by
  the extension above, requires an explicit, separate check — stated by the
  author and verified by the reviewer, not merely assumed because
  "AGENTS.md was followed" — that the diff does not contain verbatim or
  near-verbatim code from Apple's corecrypto (or the other component's)
  source (including variable/field names, comments, or distinctive structure
  copied from it). This check must be performed by reading the actual diff
  side-by-side with knowledge of corecrypto's public layout/API (never its
  source, per above), not by
  asking an AI agent to self-certify its own output. If a reviewer cannot
  confirm this, the PR must not be merged until a maintainer with corecrypto
  familiarity confirms it.

## 6. No leftover runtime validation or debugging code

Debugging aids must never be present in final commits or PRs. This includes:

- Writing to temp files, or dumping buffers/state/arguments to disk or stdout
  when some condition occurs.
- `printf`/`NSLog`/`println!`/`print` debugging, ad-hoc logging, or
  `abort()`/`assert` probes added only to observe application behavior.
- Environment-variable, file-existence, or flag-gated "diagnostic" paths that
  exist only to inspect the behavior of a particular app.
- Commented-out debugging code, `TODO remove`, or `HACK` markers left behind.

Unit tests, integration tests, and *intentional, documented* logging that is
part of the product's design are fine and encouraged. Before finishing any
task, review the full diff specifically for leftover probes.

## 7. Provenance and clean-room hygiene

- **No disassembly or decompilation of Apple binaries**, and no use of leaked
  or confidentially obtained Apple source, internal documents, or NDA-covered
  material. If you suspect something is such material, do not use it.
- Black-box observation of public API behavior is the primary way we discover
  quirks. Record findings as behavioral specifications and tests (inputs,
  outputs, OS version), not as descriptions of internals.
- Every PR must include a short **Provenance** note: what sources informed the
  change (public docs, standards, test vectors, black-box observations,
  backtraces), and confirmation that no restricted material was used.
- Keep research notes separate from implementation: describe observed behavior
  in neutral terms ("function X returns error E for input class Y"), not in
  terms of someone else's code.

### Backtraces from macOS

Backtraces captured from a real macOS system are permitted evidence and are
part of our clean-room process. They are observations of runtime behavior, not
decompilation.

**Who provides them.** A human contributor captures the backtrace on a
macOS system they are entitled to use (e.g. from a debugger, a crash report,
or a sampling tool) and supplies it as input. Agents must not attempt to
obtain backtraces by disassembling, unpacking, or otherwise inspecting Apple
binaries themselves.

**What they may be used for:**

- Learning the call hierarchy: which framework or library calls which, and
  through which public entry points (feeds Section 2).
- Learning ordering, threading and queue context, callback and delegate
  invocation paths, and where errors originate and propagate.
- Confirming that a public API is implemented on top of another public API.
- Supporting a quirk as evidence (Section 3), together with a reproducible
  test.

**What they must not be used for:**

- Reconstructing function bodies, algorithms, data structures, or control flow
  inside a function. A stack tells you who called whom, not how a function
  works.
- Using instruction offsets, register contents, raw memory dumps, or
  disassembly that may accompany a trace. If a supplied trace contains such
  material, ignore it and ask the contributor to remove it. Do not use it.
- Adding private or undocumented symbols to our public surface merely because
  they appear in a trace. Implement a private symbol only when public-layer
  behavior requires it, it is general, and a maintainer approves.
- Copying symbol names into our code as a way to mimic internal structure.
  Our internals are named and organized by our own design. Only the public API
  and the layering are dictated by compatibility.

**Hygiene for supplied traces:**

- Strip personal data, paths containing usernames, hostnames, tokens, and
  application-private information before sharing.
- State the macOS version and architecture, and how the trace was captured.
- In the PR, summarize what was learned from the trace in neutral terms
  ("API X is entered via Y, then delegates to Z on a background queue"). Do
  not commit raw traces to the repository unless a maintainer agrees.

## 8. Tests

- Every new API behavior needs tests derived from the **documented contract**,
  public standards, or (for quirks) reproducible black-box observation, not
  from one application's traffic.
- Layering-relevant behavior (e.g. a higher-level framework correctly
  delegating to the lower-level library, shared objects behaving identically
  across layers) should be covered by tests that exercise both entry points.
- Include negative and edge-case tests (error codes, invalid arguments,
  nullability, boundary sizes), especially for crypto and parsing code.
- Test vectors must come from public, license-clean sources; cite them.
- Never weaken or delete a failing test to make a change pass. Fix the cause or
  raise it.

## 9. Quality and security expectations

- No `unsafe`/unchecked memory handling without justification in a comment.
- Crypto code: constant-time comparisons for secrets, zeroize key material, no
  secrets in logs, no custom primitives.
- Add or update dependencies only if the license is compatible; list new
  dependencies and their licenses in the PR.
- Keep public API signatures binary- and source-compatible with the documented
  Apple API. Don't add public symbols that aren't part of the contract without
  maintainer sign-off.

## 10. When to stop and ask

Stop and ask a maintainer instead of proceeding when:

- Compatibility seems to require behavior that is undocumented, and the only
  way to learn it would be to look at proprietary or restricted code.
- A needed quirk has only inference-tier evidence, is security-sensitive, or
  could only be confirmed by looking at proprietary or restricted code.
- You cannot tell whether observed behavior is general or specific to one
  application.
- You cannot tell which layer a piece of functionality belongs to, or a change
  would require breaking the layering (upward dependency, cycle, or bypassing
  a layer).
- A supplied backtrace includes disassembly, register or memory contents, or
  anything that looks like it came from restricted material.
- You are unsure whether a source's license permits use.
- A fix appears to require an application-specific workaround.
- You suspect you have encountered, or produced something derived from,
  Apple's corecrypto or other restricted code.

## 11. PR checklist

- [ ] Behavior follows public documentation/standards; no app-specific hacks
- [ ] Implementation respects the macOS layering: built on the correct lower
      layer, fix placed at the right layer, no upward dependencies or cycles
- [ ] Any undocumented behavior is isolated, marked with a self-contained
      `QUIRK:` comment (behavior, evidence, test), pinned by a test, and not
      application-specific
- [ ] Any backtrace used was human-supplied, sanitized, used only for
      hierarchy/ordering/context, and summarized neutrally in the PR
- [ ] No verbatim third-party code; any reused code is licensed, attributed,
      and listed
- [ ] No corecrypto-derived code or reference, nor from any other
      source-available-but-not-free-software Apple release (Section 5)
- [ ] If this PR touches the CoreCrypto reimplementation, or any other
      component covered by Section 5's extension: a human reviewer has
      explicitly checked the diff for verbatim/near-verbatim code from
      Apple's source (not merely assumed compliance)
- [ ] The PR author can explain, in their own words, what this change does
      and why it is correct, and will personally respond (and not AI) to
      maintainer questions on this PR
- [ ] No debugging/probe code left in the diff
- [ ] Tests added from contract/standards/observation; vectors cited
- [ ] Provenance note included
- [ ] New dependencies' licenses checked

