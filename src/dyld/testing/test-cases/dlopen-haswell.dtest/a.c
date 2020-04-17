#include <stdbool.h>

bool isHaswell()
{
#if __x86_64h__
    return true;
#else
    return false;
#endif
}
