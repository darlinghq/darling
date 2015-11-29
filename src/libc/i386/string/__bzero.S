#ifdef DARLING
#	define ___bzero __bzero
#endif

	.globl ___bzero
___bzero:
	jmp _bzero@PLT
