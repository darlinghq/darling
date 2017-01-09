#include "a.h"

int  get_x_2 (void)   { return wrapper<int>::x; }
void set_x_2 (int x_) { wrapper<int>::x = x_; }
