#ifndef CGLCONTEXT_H
#define CGLCONTEXT_H

class CGLContextImpl;

struct CGLContext
{
	void* rend;
	void* disp;
	CGLContextImpl* priv;
	void* stak;
};


#endif

