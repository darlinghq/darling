#include "visibility.h"

/* GNUstep defines these as static const, Apple actually exports them */

#ifdef __x86_64__
#	define CGFloat double
#else
#	define CGFloat float
#endif

struct FLPair
{
	CGFloat a, b;
};

DARLING_VISIBLE const struct FLPair NSZeroPoint = { 0, 0 };
DARLING_VISIBLE const struct FLPair NSZeroSize = { 0, 0 };
DARLING_VISIBLE const struct FLPair NSZeroRect = { 0, 0 };

