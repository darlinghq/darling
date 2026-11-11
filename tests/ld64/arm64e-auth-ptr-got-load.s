// Copyright (C) 2026 Lubos Dolezel
//
// An arm64e function that loads the signed pointer to _twice from its
// __DATA,__auth_ptr slot through GOT_LOAD_PAGE21/PAGEOFF12 relocations, the
// way arm64e compilers have emitted function pointer loads. The slot is the
// GOT entry, so the linked code must load from it (LDR), not compute its
// address (ADD).
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_get
	.p2align	2
_get:
	adrp	x0, l_twice$auth_ptr$ia$0@GOTPAGE
	ldr	x0, [x0, l_twice$auth_ptr$ia$0@GOTPAGEOFF]
	ret
	.p2align	2
_twice:
	lsl	w0, w0, #1
	ret
	.section	__DATA,__auth_ptr
	.p2align	3
l_twice$auth_ptr$ia$0:
	.quad	_twice@AUTH(ia,0)
.subsections_via_symbols
