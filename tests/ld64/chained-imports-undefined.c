// Copyright (C) 2026 Lubos Dolezel
// Object for chained-imports.sh: imports a function and data that no linked
// dylib defines, through pointers and through calls.
extern int undefined_elsewhere(int);
extern int also_undefined;
int (*fp)(int) = undefined_elsewhere;
int* dp = &also_undefined;
int call(int v) { return undefined_elsewhere(v) + also_undefined; }
