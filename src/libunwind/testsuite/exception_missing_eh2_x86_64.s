
/* Need assembly to create bar() without eh info */

	.text
.globl _bar
_bar:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	$0, %eax
	call	_foo
	leave
	ret
