#include <stdbool.h>  // fprintf(), NULL


extern
#if __i386__
__attribute__((regparm(3)))
#endif
bool dointtest(int p1, int p2, int p3, int p4, int p5);

#if __ppc__ || __ppc64__
extern bool dofloattest(double,double,double,double,double,double,double,double,double,double,double,double,double);
#endif


#if __i386__ || __x86_64__ || __ppc__ || __ppc64__

	#if __i386__ || __x86_64__
		typedef float               vFloat  __attribute__ ((__vector_size__ (16)));
	#elif __ppc__ || __ppc64__
		typedef __vector float		vFloat;
	#endif

	extern bool dovectortest(vFloat, vFloat, vFloat, vFloat, vFloat);

#endif