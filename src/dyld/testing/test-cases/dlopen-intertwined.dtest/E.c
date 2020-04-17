extern void a(const char*);

void e() { }


void __attribute__((constructor))
initE()
{
    a("initE");
}



