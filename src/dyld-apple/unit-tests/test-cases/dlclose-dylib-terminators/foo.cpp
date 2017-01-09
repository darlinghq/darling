
extern void bar();



class A {
public:
	A() { bar(); }
	~A() { bar(); }
};


// Create global object which will have its destructor run when
// this dylib is unloaded.  The destructor will call into libbar,
// so libbar.dylib can't be unloaded before this dylib.
A a;




void foo()
{
}
