#include "common.h"
#include <stdio.h>

static bool b = false;
static bool u = false;
static bool u2 = false;
static bool isOk = true;

void setB()
{
	if ( u || b || u2 )
		isOk = false;
	b = true;
}

void setU()
{
	if ( u  || u2 )
		isOk = false;
	u = true;
}

void setU2()
{
	if ( u2 )
		isOk = false;
	u2 = true;
}

// return true iff
// setB() was called, then setU()
bool ok()
{
	//fprintf(stderr, "isOk=%d, u=%d, b=%d\n", isOk, u, b);
	return isOk && u && b && u2;
}

