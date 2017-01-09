
#if __ppc__ 

    .text

_prejunk:
    mr  r3,r5
    mr  r3,r4
    blr


_space1:
    .space 15*1024*1024 + 2
    
    .align 5
_junk:
    mr  r3,r5
    mr  r3,r4
    blr
    
    
_space2:
    .space 2*1024*1024
 
#endif


#if __arm__

    .text
_prejunk:
    mov	r0, #1
    nop

#if __thumb2__
	// thumb2 branches are +/- 16MB
_space1:
    .space 12*1024*1024
_space2:
    .space 12*1024*1024
_space3:
    .space 12*1024*1024


#elif __thumb__
	// thumb1 branches are +/- 4MB
_space1:
    .space 3*1024*1024
_space2:
    .space 3*1024*1024
_space3:
    .space 3*1024*1024
    
#else

	// ARM branches are +/- 32MB
_space1:
    .space 24*1024*1024
_space2:
    .space 24*1024*1024
_space3:
    .space 24*1024*1024

#endif

    .align 5
_junk:
    mov	r0, #1
    nop
    
    
_space4:
    .space 2*1024*1024
#endif

    .subsections_via_symbols
    