# Random-block differential vectors

28 files (14 footprints, linear and sRGB), 300 blocks each, written by
`tools/gen_random_vectors.py`. About 10% of the blocks are void-extent blocks with
random (often invalid) coordinates and reserved bits, about 70% have a random but
structured configuration (random block mode, partition count and seed, CEM selector,
the rest random) and about 20% are pure random bits. Most are illegal, so these files
exercise the error colour, HDR endpoint modes inside LDR decoding, endpoint mode 1,
mixed-class multi-partition CEMs and dual plane with several partitions.

The reference is the astcenc 5.7.1 decoder run as a black box (`-dl` or `-ds`). Only its
FNV-1a hash is stored (`fnv1a64` column), to keep the repository small; regenerate with the
`keep-rgba` argument to get the RGBA files for diffing.

Scratch runs of the same generator, 5000 blocks per vector and four seeds (about 560k
blocks), also decoded bit-exactly with this library.
