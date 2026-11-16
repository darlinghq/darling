# Finder.app project — plan roadmap

**Spec:** `docs/superpowers/specs/2026-11-16-finder-app-design.md`

The spec spans eight subsystems in five repositories. Each gets its own plan,
written just before it is executed (so it can build on what the previous
plans actually produced), and each ends in working, tested software.

| # | Plan | Repos | Depends on | Status |
|---|------|-------|-----------|--------|
| 1 | Real `unmount(2)`; `hdiutil detach` uses it | xnu, darling-dmg | — | done: `2026-11-16-unmount-syscall.md` |
| 2 | Disk-image attach library; `hdiutil` on top of it; volume naming; licence agreements in `hdiutil` | darling-dmg | 1 | done: `2026-11-16-disk-image-attach-library.md` |
| 3 | DiskArbitration: `diskarbitrationd` + client library; `hdiutil` reports DA disk names | darling (src/frameworks/DiskArbitration, new daemon), darling-dmg, darling-xnu (volume name and kind) | 1, 2 | done: `2026-11-16-disk-arbitration.md` |
| 4 | Foundation volume enumeration, NSURL volume keys, trash; NSWorkspace volumes, eject, mount notifications, `iconForFile:`, recycle, reveal | foundation, cocotron | 3 | — |
| 5 | NSCollectionView (modern API, layouts, selection, drag & drop); pasteboard file URLs | cocotron | — (parallel to 1–4) | — |
| 6 | DiskImageMounter.app (licence UI, attach, open volume) | darling | 2, 4 | — |
| 7 | Finder.app (windows, sidebar, icon/list views, `.DS_Store`, styled windows, file ops, eject, reveal) | darling | 4, 5 | — |
| 8 | Launcher "open host file", `.desktop` + MIME, packaging; end-to-end test | darling (src/startup, packaging) | 6, 7 | — |

## Common rules for every plan

- Build tree: `~/Projects/darling/build`, configured with
  `-DCMAKE_INSTALL_PREFIX=$HOME/darling-finder`; install with `make install` (no sudo).
  `/usr/local` belongs to another session — never install there.
- Test containers: `~/.darling-finder` (`DARLING_ROOTLESS=1`), `~/.darling-finder-nr`
  (`DARLING_NONROOT=1`), `~/.darling-finder-root` (root mode). The user keeps
  `~/darling-finder/bin/darling` setuid root; re-apply it (`chown root:root`, `chmod 4755`) after an install.
- Push branches but open **no pull requests** until the whole roadmap is done and the
  app works (user's rule).
- Runtime testing follows the darling-skills `darling-guest-testing` skill
  (`https://github.com/darlinghq/darling-skills`); read its `SKILL.md` first.
- Black-box comparison with real macOS: `ssh lubos@10.10.10.152` (macOS
  Tahoe). Expected values in tests that the documentation does not pin are
  confirmed there first and recorded (tier-2 evidence, AGENTS.md §3).
- Submodule changes are committed on a branch inside the submodule; the main
  repo's `finder-app` branch records the submodule bump. The main checkout has
  unrelated submodule modifications — never `git add -A` / `git commit -a`.
- Every PR carries the Provenance note (AGENTS.md §7) and the §11 checklist.
