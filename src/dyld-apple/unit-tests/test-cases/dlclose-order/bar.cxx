#include "base.h"


class Bar {
public:
	Bar() { if ( bazInitied) barInitied = true; }
	~Bar() { if ( barInitied && !bazTeminated ) barTeminated = true; }
};


Bar bar;

