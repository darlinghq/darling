/*
This file is part of Darling.

Copyright (C) 2013 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TLS_H
#define TLS_H
#include "../libmach-o/MachO.h"
#include <vector>
#include <set>
#include <pthread.h>
#include <stdint.h>

namespace Darling
{
	typedef char* ThreadData;

	struct ImageData
	{
		long key;

		// Location of initial variable values
		void* start;
		uintptr_t length;

		std::vector<void*> initializers;
	};

	struct DestructorLinkedListElement
	{
		void (*destructor)(void*);
		void* object;
		DestructorLinkedListElement *next, *first;
	};

	// Called upon image load
	void TLSSetup(void* key, std::vector<tlv_descriptor*>& descriptors, std::vector<void*>& initializers, void* start, uintptr_t length);

	// Called upon image unload
	void TLSTeardown(void* key);

	// Called by pthread upon thread destruction
	void TLSTeardownThread(ThreadData data);
	
	// Called by darling_tls_get_addr() when the variable is accessed for the first time - slow path
	void* TLSAllocate(pthread_key_t key) asm("darling_tls_allocate");
	
	// Called by object initializers to register a destructor
	void TLSAtExit(void (*destructor)(void*), void* object) asm("_tlv_atexit");
	
	// If this gets called, then the image wasn't initialized properly
	void* TLSBootstrap(tlv_descriptor* desc) asm("_tlv_bootstrap");
	
	// Implemented in assembly
	void* TLSGetAddr(tlv_descriptor* desc) asm("darling_tls_get_addr");


}

#endif

