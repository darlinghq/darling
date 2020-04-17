extern void d(const char*);

void f() {  }

void __attribute__((constructor))
initF()
{
    d("initF");
}



