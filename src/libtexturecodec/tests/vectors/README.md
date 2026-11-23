# Reference vectors

76 vectors, 14 footprints x (blobs, alpha, noise, grayalpha, normal) plus smooth and
flat at 4x4, 8x6 and 12x12. Images are 30x26, so every footprint ends in
partial blocks on at least one axis. `vectors.tsv` lists, per vector, the footprint, size,
whether sRGB operation mode was used and the astcenc arguments.

Regenerate with the astcenc 5.7.1 arm64 release binary (hash in `PROVENANCE.md`):

    python3 -I tools/gen_vectors.py /path/to/astcenc-neon tests/vectors /path/to/scratch

The `full` mode writes all 392 combinations (about 13 MB, not committed). All 392 decode
bit-exactly with this library; the committed 76 are the subset kept small on purpose.

What the reference covers (counted by `build/test_vectors` over the committed set):
normal and void-extent blocks, 1 to 4 partitions, dual plane, trit/quint/bit weights and
colour ranges, endpoint modes 0, 4, 5, 6, 8, 9, 10, 12 and 13. What it does not cover:
endpoint mode 1 (astcenc never emitted it), HDR modes and every illegal block (the
encoder emits no error blocks), dual plane together with several partitions of mixed
endpoint class. Those are checked only by hand-assembled blocks in `tests/`, i.e. against
the specification text by reading it.
