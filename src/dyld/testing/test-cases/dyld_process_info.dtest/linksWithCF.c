#include <mach/mach.h>

int main()
{
    task_suspend(mach_task_self());
    return 0;
}

