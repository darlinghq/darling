#!/bin/sh
# Mutation probes: apply one temporary edit at a time to a scratch copy of the
# library and check that the vector or unit tests fail. Usage:
#   tools/mutation_probes.sh SCRATCH_DIR
# Every line of PROBES is: file<TAB>sed expression<TAB>description.
set -u
scratch=${1:?scratch dir}
here=$(cd "$(dirname "$0")/.." && pwd)
PROBES='src/astc_decode.c	s/srgb ? 0x80 : c0/srgb ? 0x80 : 0x80/	alpha and RGB both use 0x80 in sRGB mode (no change for sRGB, breaks linear)
src/astc_decode.c	s/(srgb ? 0x80 : c0)/c0/	sRGB expansion by replication
src/astc_decode.c	s/\* i + 32) >> 6/* i + 31) >> 6/	blend rounding
src/astc_decode.c	s/ds \* x \* (gw - 1) + 32/ds * x * (gw - 1) + 31/	infill rounding
src/astc_decode.c	s/(fs \* ft + 8) >> 4/(fs * ft + 7) >> 4/	bilinear w11 rounding
src/astc_decode.c	s/c->b.ccs == k/c->b.ccs == (k ^ 1)/	CCS channel mapping
src/astc_endpoint.c	s/set_contracted(e\[0\], v\[1\], v\[3\], v\[5\], a1)/set_rgba(e[0], v[1], v[3], v[5], a1)/	no blue contraction in direct modes
src/astc_endpoint.c	s/v\[1\] + v\[3\] + v\[5\] >= v\[0\] + v\[2\] + v\[4\]/v[1] + v[3] + v[5] > v[0] + v[2] + v[4]/	direct mode comparison
src/astc_endpoint.c	s/\*a \&= 0x3F;/*a \&= 0x7F;/	bit_transfer mask
src/astc_endpoint.c	s/v\[0\] \* v\[3\] >> 8/v[0] * v[3] >> 7/	base+scale shift
src/astc_partition.c	s/14 - 4 \* i/15 - 4 * i/	partition hash offset
src/astc_partition.c	s/bw \* bh < 31/bw * bh < 30/	small-block threshold
src/astc_ise.c	s/t0 = (((c >> 1) \& 1) << 1)/t0 = (((c >> 0) \& 1) << 1)/	trit unpacking
src/astc_ise.c	s/return unq > 32 ? unq + 1 : unq;/return unq > 33 ? unq + 1 : unq;/	weight 0..64 expansion
src/astc_ise.c	s/replicate(v, r->bits, 6)/replicate(v, r->bits, 5) << 1/	weight bit replication
src/astc_block.c	s/extra = 3 \* np - 4;/extra = 3 * np - 3;/	extra CEM bit count
src/astc_block.c	s/get_bits(blk, 128 - wbits - (uint32_t)extra - 2, 2)/get_bits(blk, 128 - wbits - 2, 2)/	CCS position
src/astc_block.c	s/\*w = b + 6;/*w = b + 5;/	8x8-style block mode row
src/astc_block.c	s/wbits < 24/wbits < 23/	weight bit lower bound
src/astc_block.c	s/wbits > 96/wbits > 97/	weight bit upper bound
src/astc_block.c	s/c\[2\] >= c\[3\]/c[2] > c[3]/	void-extent t equality
src/astc_file.c	s/{ 6, 5, 5 }/{ 6, 5, 6 }/	3D footprint table entry'
detected=0
total=0
printf '%s\n' "$PROBES" | while IFS='	' read -r file expr desc; do
	work="$scratch/probe"
	mkdir -p "$work"
	cp -R "$here/src" "$here/include" "$here/tests" "$here/GNUmakefile" "$work/"
	sed -i "$expr" "$work/$file"
	if cmp -s "$work/$file" "$here/$file"; then
		echo "NOOP     $desc"
		continue
	fi
	if (cd "$work" && make -s test >/dev/null 2>&1); then
		echo "SURVIVED $desc"
	else
		echo "DETECTED $desc"
	fi
	rm -r "$work"
done
