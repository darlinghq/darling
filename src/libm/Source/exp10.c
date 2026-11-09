extern double pow(double b, double p);
extern float powf(float b, float p);

// Built with -fno-builtin (see CMakeLists.txt): otherwise the compiler rewrites pow(10, x) back
// into a call to __exp10 and this function tail-calls itself forever.
double __exp10(double x) {
	return pow(10, x);
};

float __exp10f(float x) {
	return powf(10, x);
};
