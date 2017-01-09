
 #include <unistd.h>
 
 
int main(int argc, const char* argv[]) 
{ 
	if ( argv[0] != NULL ) {
		// re-exec with empty argv[] array
		char* const emptyArgv[] = { NULL };
		execv(argv[0], emptyArgv);
	}

	return 0; 
}
