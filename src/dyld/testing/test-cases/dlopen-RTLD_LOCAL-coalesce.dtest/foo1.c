

int __attribute__((weak)) coalA = 1;
int __attribute__((weak)) coalB = 1;

int foo1_coalA()
{
    return coalA;
}

int foo1_coalB()
{
    return coalB;
}

