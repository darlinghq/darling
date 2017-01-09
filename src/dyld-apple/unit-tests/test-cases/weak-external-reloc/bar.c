

int bar[] = { 20, 21, 22, 23 };

// needs something weak to join coalescing
int __attribute__((weak)) junk = 5;
