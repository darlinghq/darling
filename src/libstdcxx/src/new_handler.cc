// Implementation file for the -*- C++ -*- dynamic memory management header.
/*
 * Copyright (c) 2009 Apple Computer, Inc. All rights reserved.
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

#include <new>
#include <cxxabi.h>

namespace std
{

const nothrow_t nothrow = {};

#ifndef _LIBCPPABI_VERSION

new_handler
set_new_handler(new_handler handler) throw()
{
    new_handler r = __cxxabiapple::__cxa_new_handler;
    __cxxabiapple::__cxa_new_handler = handler;
    return r;
}

bad_alloc::~bad_alloc() throw() { }

const char* 
bad_alloc::what() const throw()
{
  return "std::bad_alloc";
}

#endif  // _LIBCPPABI_VERSION

}  // std
