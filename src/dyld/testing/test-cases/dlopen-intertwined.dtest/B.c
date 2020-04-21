extern void c(const char*);

void b() {  }

void __attribute__((constructor))
initB()
{
    c("initB");
}


