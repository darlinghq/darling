section .text

global __error
extern __errno_location
__error:
	jmp __errno_location

