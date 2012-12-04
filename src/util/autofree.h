#ifndef AUTOFREE_H
#define AUTOFREE_H

struct autofree
{
	void operator()(void* x) { free(x); }
};

#endif

