

#include <stdbool.h>

extern bool inInitB;
extern bool doneInitB;

bool initsInWrongOrder = false;
bool doneInitA = false;

__attribute__((constructor))
void initA()
{
    if ( inInitB )
        initsInWrongOrder = true;
    doneInitA = true;
}

bool allInitsDone()
{
    return doneInitA && doneInitB;
}
