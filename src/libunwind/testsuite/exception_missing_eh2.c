

// bar() calls foo()
// this is in a separate file because we want bar() to not have eh info

extern int foo();

int global;

int bar() { 
	int x = foo();
	++global;	// to prevent tail call optimization
	return x; 
}
