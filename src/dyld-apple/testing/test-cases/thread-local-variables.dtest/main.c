

// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC main.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/thread-local-variables.exe


// RUN:  ./thread-local-variables.exe


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>


extern __thread int a;
extern __thread int b;
static __thread int c;
static __thread int d = 5;


static int* sAddr1[4];
static int* sAddr2[4];
static int* sAddr3[4];

static pthread_t sWorker1;
static pthread_t sWorker2;


// verify all initial TLV values are correct
static void checkValues()
{
    assert(a == 0);
    assert(b == 5);
    assert(c == 0);
    assert(d == 5);
}


// return addresses of all TLVs
static void getAddresses(int* array[])
{
    array[0] = &a;
    array[1] = &b;
    array[2] = &c;
    array[3] = &d;
}

static void* work2(void* arg)
{
    checkValues();
    getAddresses(sAddr2);

	return NULL;
}

static void* work1(void* arg)
{
    checkValues();

	if ( pthread_create(&sWorker2, NULL, work2, NULL) != 0 ) {
        printf("[FAIL] thread-local-variables, pthread_create\n");
		exit(0);
	}
 	void* dummy;
	pthread_join(sWorker2, &dummy);

    getAddresses(sAddr1);

	return NULL;
}

static bool someMatch(int* t1, int* t2, int* t3)
{
    if ( t1 == t2 )
        return true;
    if ( t1 == t3 )
        return true;
    if ( t2 == t3 )
        return true;
    return false;
}

int main()
{
    printf("[BEGIN] thread-local-variables\n");

    checkValues();

	if ( pthread_create(&sWorker1, NULL, work1, NULL) != 0 ) {
        printf("[FAIL] thread-local-variables, pthread_create\n");
		return 0;
	}

    getAddresses(sAddr3);

 	void* dummy;
	pthread_join(sWorker1, &dummy);

    // validate each thread had different addresses for all TLVs
    if ( someMatch(sAddr1[0], sAddr2[0], sAddr3[0]) )
        printf("[FAIL] thread-local-variables, &a is same across some threads\n");
    else if ( someMatch(sAddr1[1], sAddr2[1], sAddr3[1]) )
        printf("[FAIL] thread-local-variables, &b is same across some threads\n");
    else if ( someMatch(sAddr1[2], sAddr2[2], sAddr3[2]) )
        printf("[FAIL] thread-local-variables, &c is same across some threads\n");
    else if ( someMatch(sAddr1[3], sAddr2[3], sAddr3[3]) )
        printf("[FAIL] thread-local-variables, &d is same across some threads\n");
    else
        printf("[PASS] thread-local-variables\n");
	return 0;
}

