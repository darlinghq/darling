#include <stdio.h>
#include <stdbool.h>

#include "base.h"

static bool         wasProblem        = false;
static const char*	coal1Where        = NULL;
static int*			coal1Addr         = NULL;
static int			checkInCountCoal1 = 0;

void baseVerifyCoal1(const char* where, int* addr)
{
	//fprintf(stderr, "baseVerifyCoal1(%s, %p)\n", where, addr);
	++checkInCountCoal1;
	if ( coal1Where == NULL ) {
		coal1Where = where;
		coal1Addr = addr;
	}
	else {
		if ( addr != coal1Addr ) {
			fprintf(stderr, "coal1 resolved to different locations.  %p in %s and %p in %s\n", 
				coal1Addr, coal1Where, addr, where);
			wasProblem = true;
		}	
	}
}


static const char*	coal2Where        = NULL;
static int*			coal2Addr         = NULL;
static int			checkInCountCoal2 = 0;

void baseVerifyCoal2(const char* where, int* addr)
{
	//fprintf(stderr, "baseVerifyCoal2(%s, %p)\n", where, addr);
	++checkInCountCoal2;
	if ( coal2Where == NULL ) {
		coal2Where = where;
		coal2Addr = addr;
	}
	else {
		if ( addr != coal2Addr ) {
			fprintf(stderr, "coal2 resolved to different locations.  %p in %s and %p in %s\n", 
				coal2Addr, coal2Where, addr, where);
			wasProblem = true;
		}	
	}
}



void baseCheck()
{
	if ( wasProblem )
        printf("[FAIL] weak-coalesce: was problem\n");
    else if ( checkInCountCoal1 != 4 )
        printf("[FAIL] weak-coalesce: checkInCountCoal1 != 4\n");
    else if ( checkInCountCoal2 != 4 )
        printf("[FAIL] weak-coalesce: checkInCountCoal2 != 2\n");
	else
        printf("[PASS] weak-coalesce\n");
}

