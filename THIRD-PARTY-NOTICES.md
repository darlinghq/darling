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
