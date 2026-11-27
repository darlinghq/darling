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

## Adwaita icon theme, `src/external/cocotron/AppKit/SystemSymbols` and `SystemImages`

- Copyright the GNOME Project, <https://www.gnome.org>.
- Source: adwaita-icon-theme, <https://gitlab.gnome.org/GNOME/adwaita-icon-theme>. The 54 SVG files
  in `SystemSymbols` are unmodified copies of its symbolic icons; the PNG files in `SystemImages` are
  icons of it rendered with `rsvg-convert` (the versions are listed in its `COPYING.Adwaita`).
- Licence: GNU LGPL version 3, or Creative Commons Attribution-Share Alike 3.0 United States
  (`COPYING.Adwaita` in each of the two directories).
- Use: AppKit draws the system symbols (`NSImage` with a symbol name) that have no drawing of their
  own from the SVG files, and uses the PNG files as the standard system images (folder, computer,
  trash, generic document icons and so on). Both are installed in the AppKit framework resources
  together with their `COPYING.Adwaita`.

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

## PipeWire and libspa headers, host/runtime dependencies of optional PipeWire backend

- What: The optional PipeWire camera/audio/screen-capture backend links at build/runtime against host-installed PipeWire/libspa libraries and uses their public headers during compilation (`libpipewire-0.3`, `spa-0.2`). No PipeWire/libspa source code is vendored in this repository.
- Upstream: PipeWire, <https://gitlab.freedesktop.org/pipewire/pipewire>.
- Licence: PipeWire is distributed under the MIT license; libspa headers/code in PipeWire are also MIT-licensed. Exact installed versions and license texts are provided by the host distribution packages used to build/run the backend.
- Use: Optional backend integration only. Builds without PipeWire development/runtime libraries disable this backend. Distributions packaging Darling with PipeWire enabled should include the corresponding PipeWire/libspa copyright and license notices from their packages.
- HDR note: HDR screen/camera capture negotiates PipeWire raw video formats/colorimetry (e.g. P010/PQ/BT2020) only when supported by the linked runtime (PipeWire >= 1.6 gate in code). No PipeWire source is vendored; behavior depends on host PipeWire/libspa and compositor/portal capabilities.
