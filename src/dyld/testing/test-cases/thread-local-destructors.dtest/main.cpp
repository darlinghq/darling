

// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CXX main.cpp -std=c++11 $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/thread-local-destructors.exe


// RUN:  ./thread-local-destructors.exe


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#include <TargetConditionals.h>

static pthread_t sMainThread;
static pthread_t sWorker1;
static pthread_t sWorker2;

static bool sMainThreadInitialized = false;
static bool sWorker1Initialized = false;
static bool sWorker2Initialized = false;
static bool sMainThreadFinalized = false;
static bool sWorker1Finalized = false;
static bool sWorker2Finalized = false;

struct Struct {
    Struct() : sInitializer(getInitializer()), sFinalizer(getFinalizer()) {
        sInitializer = true;
    }
    ~Struct() {
        sFinalizer = true;
    }

    bool& getInitializer() {
        if (pthread_equal(pthread_self(), sMainThread)) {
            return sMainThreadInitialized;
        }
        if (pthread_equal(pthread_self(), sWorker1)) {
            return sWorker1Initialized;
        }
        if (pthread_equal(pthread_self(), sWorker2)) {
            return sWorker2Initialized;
        }
        assert(false);
    }

    bool& getFinalizer() {
        if (pthread_equal(pthread_self(), sMainThread)) {
            return sMainThreadFinalized;
        }
        if (pthread_equal(pthread_self(), sWorker1)) {
            return sWorker1Finalized;
        }
        if (pthread_equal(pthread_self(), sWorker2)) {
            return sWorker2Finalized;
        }
        assert(false);
    }

    // TLVs are laxily initialized so have something to do to trigger init
    void doWork() {
    }

    bool& sInitializer;
    bool& sFinalizer;
};

static thread_local Struct s;

// Add another thread local so that we test dyld's ability to grow the vector of tlv atexits to run

static bool sMainThreadInitialized_Another = false;
static bool sWorker1Initialized_Another = false;
static bool sWorker2Initialized_Another = false;
static bool sMainThreadFinalized_Another = false;
static bool sWorker1Finalized_Another = false;
static bool sWorker2Finalized_Another = false;

struct AnotherStruct {
    AnotherStruct() : sInitializer(getInitializer()), sFinalizer(getFinalizer()) {
        sInitializer = true;
    }
    ~AnotherStruct() {
        sFinalizer = true;
    }

    bool& getInitializer() {
        if (pthread_equal(pthread_self(), sMainThread)) {
            return sMainThreadInitialized_Another;
        }
        if (pthread_equal(pthread_self(), sWorker1)) {
            return sWorker1Initialized_Another;
        }
        if (pthread_equal(pthread_self(), sWorker2)) {
            return sWorker2Initialized_Another;
        }
        assert(false);
    }

    bool& getFinalizer() {
        if (pthread_equal(pthread_self(), sMainThread)) {
            return sMainThreadFinalized_Another;
        }
        if (pthread_equal(pthread_self(), sWorker1)) {
            return sWorker1Finalized_Another;
        }
        if (pthread_equal(pthread_self(), sWorker2)) {
            return sWorker2Finalized_Another;
        }
        assert(false);
    }

    // TLVs are laxily initialized so have something to do to trigger init
    void doWork() {
    }

    bool& sInitializer;
    bool& sFinalizer;
};

static thread_local AnotherStruct sAnotherStruct;

static void* work2(void* arg)
{
    s.doWork();

	return NULL;
}

static void* work1(void* arg)
{
    s.doWork();

	if ( pthread_create(&sWorker2, NULL, work2, NULL) != 0 ) {
        printf("[FAIL] thread-local-destructors, pthread_create\n");
		exit(0);
	}
 	void* dummy;
	pthread_join(sWorker2, &dummy);

	return NULL;
}

bool passedChecksInMain = false;

void checkMainThreadFinalizer() {
    if ( !passedChecksInMain )
        return;
    // _tlv_exit is only called on x86 mac
#if TARGET_OS_OSX
    bool shouldFinalize = true;
#else
    bool shouldFinalize = false;
#endif
    if ( sMainThreadFinalized != shouldFinalize )
        printf("[FAIL] thread-local-destructors, main thread finalisation not as expected\n");
    else if ( sMainThreadFinalized_Another != shouldFinalize )
        printf("[FAIL] thread-local-destructors, main thread other struct finalisation not as expected\n");
    else
        printf("[PASS] thread-local-destructors\n");
}

int main()
{
    printf("[BEGIN] thread-local-destructors\n");

    sMainThread = pthread_self();
    s.doWork();

    sAnotherStruct.doWork();

    // Note, as of writing this is being run after the tlv finalizer for this thread
    // so this should check that this order continues to be used.
    atexit(&checkMainThreadFinalizer);

	if ( pthread_create(&sWorker1, NULL, work1, NULL) != 0 ) {
        printf("[FAIL] thread-local-destructors, pthread_create\n");
		return 0;
	}

 	void* dummy;
	pthread_join(sWorker1, &dummy);

    // validate each thread had different addresses for all TLVs
    if ( !sMainThreadInitialized )
        printf("[FAIL] thread-local-destructors, main thread was not initialized\n");
    else if ( !sWorker1Initialized )
        printf("[FAIL] thread-local-destructors, thread 1 was not initialized\n");
    else if ( !sWorker2Initialized )
        printf("[FAIL] thread-local-destructors, thread 2 was not initialized\n");
    else if ( !sWorker1Finalized )
        printf("[FAIL] thread-local-destructors, thread 1 was not finalised\n");
    else if ( !sWorker2Finalized )
        printf("[FAIL] thread-local-destructors, thread 2 was not finalised\n");
    else if ( !sMainThreadInitialized_Another )
        printf("[FAIL] thread-local-destructors, main thread other variable was not initialized\n");
    else if ( !sWorker1Initialized_Another )
        printf("[FAIL] thread-local-destructors, thread 1 other variable was not initialized\n");
    else if ( !sWorker2Initialized_Another )
        printf("[FAIL] thread-local-destructors, thread 2 other variable was not initialized\n");
    else if ( !sWorker1Finalized_Another )
        printf("[FAIL] thread-local-destructors, thread 1 other variable was not finalised\n");
    else if ( !sWorker2Finalized_Another )
        printf("[FAIL] thread-local-destructors, thread 2 other variable was not finalised\n");
    else
        passedChecksInMain = true;

	return 0;
}

