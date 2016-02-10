
//
// TEST-OPTIONS:  -arch i386 -Wl,-no_compact_unwind
// TEST-OPTIONS:  -arch x86_64 -Wl,-no_compact_unwind
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <mach/mach.h>
#include <libunwind.h>

#include "unwind.h"

// private keymgr stuff
#define KEYMGR_GCC3_DW2_OBJ_LIST   302  
extern "C" {
	extern void	 _keymgr_set_and_unlock_processwide_ptr(int key, void* ptr);
	extern void* _keymgr_get_and_lock_processwide_ptr(int key);
};


// undocumented libgcc "struct object"
struct libgcc_object 
{
	void*			start;
	void*			unused1;
	void*			unused2;
	void*			fde;
	unsigned long	encoding;
	void*			fde_end;
	libgcc_object*	next;
};

// undocumented libgcc "struct km_object_info" referenced by KEYMGR_GCC3_DW2_OBJ_LIST
struct libgcc_object_info {
  struct libgcc_object*		seen_objects;
  struct libgcc_object*		unseen_objects;
  unsigned					spare[2];
};


// Some application have reverse engineer libgcc and do the equivalent
// of my_keymgr_register_frame() to register dynamically created code.
static void my_keymgr_register_frame(void* fde)
{
	// acquire exclusive access to libgcc_object_info
	libgcc_object_info* head = (libgcc_object_info*)_keymgr_get_and_lock_processwide_ptr(KEYMGR_GCC3_DW2_OBJ_LIST);

	// create new object
	libgcc_object* ob = (libgcc_object*)calloc(sizeof(libgcc_object), 1);
	ob->fde = fde;
	
	// insert new object into linked list
	ob->next = head->unseen_objects;
	head->unseen_objects = ob;

	// release libgcc_object_info 
	_keymgr_set_and_unlock_processwide_ptr(KEYMGR_GCC3_DW2_OBJ_LIST, head);
}



void check(bool shouldFindMain)
{
	unw_cursor_t cursor; 
	unw_context_t uc;
	unw_word_t	offset;
	bool		foundMain = false;
	char	functionName[256];
	//fprintf(stderr, "check(shouldFindMain=%d)\n", shouldFindMain);
	
	int level = 1;
	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);
	do {
		unw_get_proc_name(&cursor, functionName, 64, &offset);
		//fprintf(stderr, "level=%d in function: %s\n", level, functionName);
		if ( strcmp(functionName, "main") == 0 )
			foundMain = true;
		++level;
		strcpy(functionName, "junk");
	} while (unw_step(&cursor) > 0);
	
	if ( foundMain == shouldFindMain )
		return;
	
	fprintf(stderr, "failing because shouldFindMain=%d and foundMain=%d\n", shouldFindMain, foundMain);
	exit(1);
}




void foo(bool shouldFindMain, void (*func)(bool))
{
	(*func)(shouldFindMain);
	__asm__ volatile("");  // disable tail call optimization
}

typedef void (*checkFuncProc)(bool shouldFindMain, void (*func)(bool));


int main()
{
	struct dwarf_eh_bases junk;
	const void* foo_eh_src = _Unwind_Find_FDE((void*)((long)&foo + 1), &junk);
	// copy foo and its fde to a dynamically allocated buffer
	unsigned long deltaToFDE = (uintptr_t)foo_eh_src - (uintptr_t)&foo;
	unsigned long size = deltaToFDE + 100;
	size = ((size + 4095) & (-4096));
	vm_address_t addr = 0;
	if ( vm_allocate(mach_task_self(), &addr, size, VM_FLAGS_ANYWHERE) != KERN_SUCCESS )
		exit(1);
	memcpy((void*)addr, (void*)&foo, size);
	mprotect((void*)addr, size, PROT_READ|PROT_EXEC);
	// make pointer to copy of foo and copy of fde
	checkFuncProc checkFunc = (checkFuncProc)addr;
	void* foo_eh = (void*)(addr + deltaToFDE);

	// call check() with unwind info unregistered, verify fails
	(*checkFunc)(false, check);
	
	// call check() with unwind info registered, verify succeeds
	__register_frame(foo_eh);
	(*checkFunc)(true, check);
	
	// call check() with unwind info unregistered, verify fails
	__deregister_frame(foo_eh);
	(*checkFunc)(false, check);
	 
	// call check() with unwind info added via keymgr, verify succeeds
	my_keymgr_register_frame(foo_eh);
	(*checkFunc)(true, check);
	
	return 0;
}

