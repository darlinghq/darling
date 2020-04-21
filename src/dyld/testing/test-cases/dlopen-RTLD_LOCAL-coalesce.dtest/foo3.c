

int __attribute__((weak)) coalA = 3;
int __attribute__((weak)) coalB = 3;
int __attribute__((weak)) coalC = 3;

int foo3_coalA()
{
    return coalA;
}

int foo3_coalB()
{
    return coalB;
}

int foo3_coalC()
{
    return coalC;
}
