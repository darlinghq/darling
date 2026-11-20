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
