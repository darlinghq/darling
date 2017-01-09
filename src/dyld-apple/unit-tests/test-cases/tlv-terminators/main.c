/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <pthread.h>
#include <stdbool.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


struct thread_var_info
{
    pthread_t th;
    int*    a_addr;
    int*    b_addr;
    bool    a_terminated;
    bool    b_terminated;
};

struct thread_var_info threadmain;
struct thread_var_info thread1;
struct thread_var_info thread2;


__thread int a;         // statically, initially 0
__thread int b = 5;     // statically, initially 5

extern void _tlv_atexit(void (*termfunc)(void* objAddr), void* objAddr);

void myinit()
{
	a = 11;      // dynamically initialized to 11
	b = 42;      // dynamically initialized to 42
}

void myterm(void* objAddr)
{
    pthread_t self = pthread_self();
    //fprintf(stderr, "myterm(%p), self=%p\n", objAddr, self);
    if ( thread1.th == self && thread1.a_addr == objAddr )
        thread1.a_terminated = true;
    else if ( thread1.th == self && thread1.b_addr == objAddr )
        thread1.b_terminated = true;
    else if ( thread2.th == self && thread2.a_addr == objAddr )
        thread2.a_terminated = true;
    else if ( thread2.th == self && thread2.b_addr == objAddr )
        thread2.b_terminated = true;
    else if ( threadmain.th == self && threadmain.a_addr == objAddr )
        threadmain.a_terminated = true;
    else if ( threadmain.th == self && threadmain.b_addr == objAddr )
        threadmain.b_terminated = true;
}

static void* work(void* arg)
{
	if ( a != 11 ) {
		FAIL("tlv-terminators: a not initialized to 11");
		exit(0);
	}
	if ( b != 42 ) {
		FAIL("tlv-terminators: b not initialized to 42");
		exit(0);
	}
    struct thread_var_info* s = (struct thread_var_info*)arg;
    s->th = pthread_self();
    s->a_addr = &a;
    s->b_addr = &b;
    s->a_terminated = false;
    s->b_terminated = false;
	//fprintf(stderr, "self=%p, arg=%p, &a=%p, &b=%p\n", s->th, arg, s->a_addr , s->b_addr);

    _tlv_atexit(myterm, &a);
    _tlv_atexit(myterm, &b);
   
	return NULL;
}

int main()
{
	pthread_t worker1;
	if ( pthread_create(&worker1, NULL, work, &thread1) != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}

	pthread_t worker2;
	if ( pthread_create(&worker2, NULL, work, &thread2) != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}

	void* result;
	//fprintf(stderr, "waiting for worker 1\n");
	pthread_join(worker1, &result);
	//fprintf(stderr, "waiting for worker 2\n");
	pthread_join(worker2, &result);

	work(&threadmain);

	//fprintf(stderr, "thread1: &a=%p, &b=%p\n", thread1.a_addr, thread1.b_addr);
	//fprintf(stderr, "thread2: &a=%p, &b=%p\n", thread2.a_addr, thread2.b_addr);
	//fprintf(stderr, "threadm: &a=%p, &b=%p\n", threadmain.a_addr, threadmain.b_addr);

    if ( ! thread1.a_terminated ) {
		FAIL("tlv-terminators: terminator for a on thread 1 not run");
		exit(0);
    }
    if ( ! thread1.b_terminated ) {
		FAIL("tlv-terminators: terminator for b on thread 1 not run");
		exit(0);
    }

    if ( ! thread2.a_terminated ) {
		FAIL("tlv-terminators: terminator for a on thread 2 not run");
		exit(0);
    }
    if ( ! thread2.b_terminated ) {
		FAIL("tlv-terminators: terminator for b on thread 2 not run");
		exit(0);
    }

   if ( threadmain.a_terminated ) {
		FAIL("tlv-terminators: terminator for a on main thread run early");
		exit(0);
    }
    if ( threadmain.b_terminated ) {
		FAIL("tlv-terminators: terminator for b on main thread run early");
		exit(0);
    }


	PASS("tlv-terminators");
	return EXIT_SUCCESS;
}
