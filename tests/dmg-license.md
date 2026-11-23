# Licence agreements in disk images

`dmg-license.sh` checks the licence agreement ("SLA") API of `libdarling-dmg` (`dmg-license.c`) and how
`hdiutil attach` asks for agreement.

```sh
DARLING_SKILLS=<darling-skills checkout> tests/dmg-license.sh
```

(Same environment as `dmg-attach.sh`, see `dmg-attach.md`.)

## Fixtures

`fixtures/make-sla-fixture.py` writes a resource file; on macOS 26.7.3 `hdiutil udifrez <image> -xml <file>` embeds
it into a copy of `fixtures/unmount-test.dmg` (the command is deprecated but works; the image comes first):

| Fixture | Generator arguments | Content |
|---------|---------------------|---------|
| `sla-test.dmg` | `5000 good` | English (region 0, Mac Roman), German (region 3, Mac Roman, non-ASCII text and prompt), Japanese (region 14, Shift JIS) |
| `sla-short-lpic.dmg` | `5000 short-lpic` | as above, the `LPic` table cut off inside its first entry |
| `sla-overcount.dmg` | `5000 overcount` | as above, `LPic` claims five entries but has three |
| `sla-lines.dmg` | `5000 lines` | as `sla-test.dmg`, English text `A<CR>B<CR><CR>C<LF>D<CR><LF>` |
| `sla-one.dmg` | `5000 one` | English only |
| `sla-rtf.dmg` | `5000 rtf` | English only, the agreement as `RTF ` (`{\rtf1\ansi SLA TEST AGREEMENT {\b bold}\par}`) |
| `sla-nostrings.dmg` | `5000 nostrings` | the three languages of `sla-test.dmg` without their `STR#` resources |
| `styled-sla-test.dmg` | `5000 one` | English only, embedded into the styled image of `make-styled-fixture.py` (volume "Styled Test") by `make-styled-fixture.py styled-sla-test.dmg --license <xml>`; `open-disk-image.sh` |

The resource layout follows public descriptions of the UDIF licence resources: `LPic` 5000 holds the default
language and a table of (region code, resource ID - 5000, two-byte flag); per language a `TEXT` (or `RTF `) with
the agreement in the language's Mac encoding and a `STR#` with the language name, Agree, Disagree, Print, Save
and the prompt.

## Region codes

`CFLocaleCreateCanonicalLocaleIdentifierFromScriptManagerCodes(NULL, -1, region)` on macOS 26.7.3 for regions
0-109 gives the language table in `LicenseAgreement.cpp`, e.g. 0 `en_US`, 3 `de_DE`, 14 `ja_JP`, 62 `uk_UA`,
71 `pt_BR` (codes with no or a generic answer, such as 28 `_CA` or 55 `und`, are not in the table).

## Observed on macOS 26.7.3 (25G342), x86_64, with `sla-test.dmg`

The user's languages were `en-US`, `cs-CZ`. With stdin not a terminal:

| Case | stdout | stderr | exit | mounted |
|------|--------|--------|------|---------|
| `hdiutil attach <image> </dev/null` | the English agreement text | `hdiutil: attach canceled` | 1 | no |
| answer line `y`, `Y`, `yes`, `Yes`, `y ` or ` y` | the text, then the usual attach output | | 0 | yes |
| answer line `n`, `N`, `nope` or an empty line | the text | `hdiutil: attach canceled` | 1 | no |
| answer lines `x`, then `y` | the text, then the usual attach output | | 0 | yes |
| answer line `x` or `agree`, then end of input; or `x`, then `n` | the text | `hdiutil: attach canceled` | 1 | no |
| `-quiet`, with any of the answers above (including `y`) | | | 1 | no |
| `-plist </dev/null` | the text (not the plist) | `hdiutil: attach canceled` | 1 | no |
| `-plist`, answer `y` | the text, then the plist | | 0 | yes |
| `LANG=de_DE.UTF-8 hdiutil attach <image> </dev/null` | the English text | (not recorded) | (not recorded) | no |
| `sla-short-lpic.dmg` or `sla-overcount.dmg </dev/null` | the usual attach output, no agreement | | 0 | yes |

No prompt text is printed when stdin is not a terminal. In a terminal the text is shown in a pager (`(END)`),
followed by the prompt `Agree Y/N? ` (this case was not automated: `script` runs of it hang on that host).
Darling prints the text without a pager and then the same prompt; that case is not covered by `dmg-license.sh`.

The text is printed reflowed: a single line break (CR or LF) becomes a space, a run of two or more starts a new
paragraph (an empty line), and the output ends with a line break. `TEXT` data `A\rB\r\rC\nD\r\n` prints as
`A B\n\nC D\n\n`; `...(English)\rThis is the English test agreement.\r` prints as
`...(English) This is the English test agreement.\n`.

Changing the first `LPic` field (5000, 0, 3, 5001) did not change the language shown: the user's language wins.
