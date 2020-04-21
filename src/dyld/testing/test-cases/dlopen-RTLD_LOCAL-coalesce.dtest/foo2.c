

int __attribute__((weak)) coalA = 2;
int __attribute__((weak)) coalB = 2;
int __attribute__((weak)) coalC = 2;

int foo2_coalA()
{
    return coalA;
}

int foo2_coalB()
{
    return coalB;
}

int foo2_coalC()
{
    return coalC;
}
