
void* test_function(void* arg1, int arg2, long long arg3);
const char* library_name = "libpulse.so.5";

void produce_stubs()
{
	STUBIFY(test_function);
}

