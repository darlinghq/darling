#ifdef DARLING
#	define ___bzero __bzero
#	define _bzero bzero
#endif

	.globl ___bzero
___bzero:
	jmp _bzero@PLT
