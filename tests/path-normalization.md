# Unicode normalization in file names

`tests/src/path_normalization.c` checks that a file name given in one Unicode normalization form finds a file stored
in the other, as on macOS. It is plain C and builds for the guest and on macOS, where it passes too.

    build/tmp/helpers/src-test.sh <mode> tests/src/path_normalization.c          # a new directory in the guest's TMPDIR
    ARGS=/Volumes/SystemRoot/<host dir> build/tmp/helpers/src-test.sh <mode> tests/src/path_normalization.c

or, by hand, `path_normalization <new directory>` in the guest. It prints PASS/FAIL lines and exits 1 on a failure.

Cases:
- a file created with the precomposed name `ü.txt` (U+00FC) is found by `access`, `stat` (same inode) and `open` with
  the decomposed name (`u` + U+0308);
- creating the decomposed name with `O_CREAT | O_EXCL` fails with `EEXIST`;
- a decomposed path through a precomposed folder (`Dïr/é.txt`) opens the file;
- `v.txt` and decomposed `ö.txt` are not found (`ENOENT`);
- a name that is not valid UTF-8 (`\xff.txt`) is found by its bytes. macOS does not take such names (`EILSEQ`), so
  there it prints SKIP.

The comparison itself has a host unit test in xnu, `darling/tests/name-equivalence.c` (see its `.md`).

## Observed on macOS

On 10.14.4 (APFS) and 26.7.3 (APFS), x86_64:
- `-[NSString fileSystemRepresentation]` of `ü.txt`, precomposed or decomposed, is decomposed: `75 cc 88 2e 74 78 74`.
- `+[NSURL fileURLWithPath:]` of the precomposed path gives `u%CC%88.txt` in `absoluteString`; its
  `fileSystemRepresentation` is decomposed too.
- A file created with the precomposed bytes is found by `access()` with the decomposed bytes, and by
  `fileExistsAtPath:` with either form. `contentsOfDirectoryAtPath:` returns the name as it was created (precomposed).
- With a precomposed `ü.txt` present, creating the decomposed name with `O_CREAT | O_EXCL` fails with `EEXIST`, and
  the directory has one entry.
- `path_normalization` passes on both (the invalid UTF-8 case is SKIP: `EILSEQ`).

So Foundation decomposes on macOS as it does in Darling, and the file system makes either form work: Apple documents
APFS as normalization-insensitive ("Apple File System Guide", FAQ). Darling's path lookup does the same. It tries the
exact name first. When that is not found, it scans the directory for a name that is equal ignoring ASCII case, or
whose canonical decomposition is (both valid UTF-8). dyld's own lookup stays exact.
