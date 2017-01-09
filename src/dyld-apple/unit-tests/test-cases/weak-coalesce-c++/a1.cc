#include "a.h"

int  get_x_1 (void)   { return wrapper<int>::x; }
void set_x_1 (int x_) { wrapper<int>::x = x_; }
