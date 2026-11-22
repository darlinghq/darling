# Third-party material in this repository

Submodules under `src/external` carry their own licences, as noted within each of them. Material
that is part of this repository itself and not under the GNU GPL 3.0 is listed here.

## Nimbus Sans (URW++ base 35 fonts), `src/fonts/nimbus-sans`

- Copyright 2014 by (URW)++ Design & Development.
- Source: <https://github.com/ArtifexSoftware/urw-base35-fonts>, commit
  3c0ba3b5687632dfc66526544a4e811fe0ec0cd9; the four faces Regular, Bold, Italic and Bold Italic of
  Nimbus Sans, unmodified.
- Licence: GNU Affero General Public License version 3 with a special exception for embedding the
  fonts in Postscript or PDF documents (`src/fonts/nimbus-sans/COPYING` and `LICENSE`). The fonts
  are separate data files, not linked into any Darling program.
- Use: at build time `src/fonts/make-helvetica-neue.py` makes `HelveticaNeue.ttc` from them, a
  renamed derivative that is installed as `/System/Library/Fonts/HelveticaNeue.ttc` in the prefix,
  with the licence texts and `src/fonts/HelveticaNeue-NOTICE.txt` under
  `usr/share/licenses/darling-helvetica-neue`.

## Adwaita icon theme, `src/Finder/Finder.iconset` and `src/DiskImageMounter/DiskImageMounter.iconset`

- Copyright the GNOME Project, <https://www.gnome.org>.
- Source: adwaita-icon-theme 51.0, <https://gitlab.gnome.org/GNOME/adwaita-icon-theme>; the icons
  `places/folder` (Finder) and `devices/drive-removable-media` (DiskImageMounter), rendered with
  `rsvg-convert` at each size of the iconset.
- Licence: GNU LGPL version 3, or Creative Commons Attribution-Share Alike 3.0 United States
  (`src/Finder/COPYING.Adwaita`, `src/DiskImageMounter/COPYING.Adwaita`).
- Use: the build makes each application's `.icns` from its iconset, installed in the application
  bundle in the prefix; DiskImageMounter's images are also installed on the host in the hicolor icon
  theme as `org.darlinghq.Darling.DiskImageMounter.png`.

## ASTC reference vectors, `src/libtexturecodec/tests/vectors` and `vectors-random`

- What: test data that is output of the astcenc 5.7.1 release binary
  (<https://github.com/ARM-software/astc-encoder>, licence Apache-2.0), run unmodified as a black box
  on synthetic inputs made by the scripts in `src/libtexturecodec/tools` (no third-party content): the
  `.astc` and `.rgba` files of `vectors` are its compression and decompression, and `vectors-random`
  holds our own random blocks with the hash of its decompression. Its source code was not read and
  none of its code or binary is in this repository. The URL of the release zip and the sha256 of the
  zip and of the binary are in `src/libtexturecodec/PROVENANCE.md`.
- Licence: no copyright holder or year is claimed for the files. Whether Apache-2.0 section 4
  requires the licence text of the tool to be shipped with output of this kind is an open question
  for the maintainers (`src/libtexturecodec/maintainer-question.md`, item 13); the licence text is not
  copied here.
- Use: test inputs of the ASTC decoder and of `tests/src/imageio_astc_source.c`; not linked into or
  installed with any Darling program.

## swift-crypto and what it vendors, `src/external/swift-crypto`

`CryptoKit.framework` is compiled from a checkout of apple/swift-crypto (the submodule) by the overlay in
darling-swift (`overlays/CryptoKit`). The submodule holds the pristine upstream files; the build changes a copy of
the Swift files as `overlays/CryptoKit/DARLING-CHANGES.md` lists. The licence texts are `LICENSE.txt` and `NOTICE.txt`
at the top level of the submodule; the table of components and their evidence is `overlays/CryptoKit/NOTICE.md` in
darling-swift.

- swift-crypto 5.1.2, commit `3280a5f72874d2f73bf25551bbb64f524c76d834`, <https://github.com/apple/swift-crypto>.
  Licence: Apache-2.0. Copyright: the SwiftCrypto Project (`NOTICE.txt`).
- BoringSSL as vendored by swift-crypto (`Sources/CCryptoBoringSSL`), commit
  `0226f30467f540a3f62ef48d453f93927da199b6`. Licence: Apache-2.0; the OpenSSL-derived files carry the OpenSSL
  Project Authors' Apache-2.0 header. It includes assembly generated from Perl scripts that carry the same
  header. Two files say they adapt public-domain SUPERCOP code.
- fiat-crypto as used by BoringSSL (`third_party/fiat`). Licence: Apache-2.0, per BoringSSL's
  `third_party/fiat/LICENSE` at the commit above.
- XKCP (`Sources/CXKCP`). Licence: CC0-1.0 for 13 of its 17 sources. `FIPS202-opt64/brg_endian.h` is Copyright
  (c) 1998-2008 Brian Gladman, Worcester, UK, under a BSD-style licence whose clause 2 requires binary
  distributions to include its notice; the full text is quoted in `overlays/CryptoKit/NOTICE.md`.
- Use: linked into the built `CryptoKit.framework`. Whether the licence texts and `NOTICE.txt` are installed with
  the framework, as the Helvetica Neue entry above does, is an open question for the maintainers.

## FEX, `src/startup/mldr/fex/darling_fexcore_*` and the `libdarling-fexcore.so` shim

- Copyright (c) 2019 Ryan Houdek <Sonicadvance1@gmail.com>.
- Source: <https://github.com/FEX-Emu/FEX>, commit 8e95a994a.
- Adapted into this repository: `darling_fexcore_signals.cpp` and `darling_fexcore_sigframe.cpp` (from
  `Source/Tools/LinuxEmulation/LinuxSyscalls/SignalDelegator.cpp` and
  `SignalDelegator/GuestFramesManagement.cpp`: guest signal frame setup and restore, the host context
  saved around a guest handler, signal deferral), `darling_fexcore_sigstack.cpp` (the per-thread host
  signal stack that a handler finds its thread from, after `SignalDelegator::RegisterTLSState`) and
  `darling_fexcore_adapter.cpp` (the call-return stack and GDT setup of a new thread, after
  `Source/Tools/LinuxEmulation/LinuxSyscalls/ThreadManager.cpp`).
- Compiled into the shim from the FEX tree it is built against, unmodified: FEXCore's static libraries,
  `FEXCore/Source/Utils/AllocatorHooks.cpp`, `Source/Common/HostFeatures.cpp`, `Source/Common/CPUInfo.cpp`,
  `Source/Common/Linux/LinuxVersion.cpp`, and the headers `Source/Tools/LinuxEmulation/ArchHelpers/MContext.h`
  and `UContext.h`. FEXCore links FEX's bundled cephes (BSD, `External/cephes/LICENSE`) and Berkeley
  SoftFloat 3e (BSD 3-clause, notice in its source files).
- Shipped: a build with `-DWITH_FEXCORE=ON` installs this file, FEX's `LICENSE` and cephes' `LICENSE`
  under `usr/share/licenses/darling-fexcore` in the Darling prefix, next to the shim's package contents.
- Licence: MIT:

  > Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
  > associated documentation files (the "Software"), to deal in the Software without restriction,
  > including without limitation the rights to use, copy, modify, merge, publish, distribute,
  > sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
  > furnished to do so, subject to the following conditions:
  >
  > The above copyright notice and this permission notice shall be included in all copies or
  > substantial portions of the Software.
  >
  > THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
  > NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  > NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
  > OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  > CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
