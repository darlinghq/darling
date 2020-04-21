#include <stddef.h>


typedef void (*NotifyProc)(void);

NotifyProc gNotifer = NULL;


static __attribute__((destructor))
void myTerm()
{
    if ( gNotifer )
        gNotifer();
}



