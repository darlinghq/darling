// -*- C++ -*- std::terminate, std::unexpected and friends.
// Copyright (C) 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002
// Free Software Foundation
//
// This file is part of GCC.
//
// GCC is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// GCC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GCC; see the file COPYING.  If not, write to
// the Free Software Foundation, 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA. 

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

#include <stdlib.h>
#include <cxxabi.h>
#include <stdio.h>

#include <exception>

#ifndef _LIBCPPABI_VERSION

void
std::terminate ()
{
  try {
    (*__cxxabiapple::__cxa_terminate_handler)();
    ::abort ();
  } 
  catch (...) {
    ::abort ();
  }
}

void
std::unexpected ()
{
	(*__cxxabiapple::__cxa_unexpected_handler)();
	std::terminate();
}

std::terminate_handler std::set_terminate(std::terminate_handler func) throw()
{
  std::terminate_handler old = __cxxabiapple::__cxa_terminate_handler;
  __cxxabiapple::__cxa_terminate_handler = func;
  return old;
}

std::unexpected_handler std::set_unexpected (std::unexpected_handler func) throw()
{
  std::unexpected_handler old = __cxxabiapple::__cxa_unexpected_handler;
  __cxxabiapple::__cxa_unexpected_handler = func;
  return old;
}

#endif  // _LIBCPPABI_VERSION

namespace __gnu_cxx
{
	void __verbose_terminate_handler()
	{
		if ( __cxxabiv1::__cxa_current_exception_type() != NULL )
			fprintf(stderr, "terminate called throwing an exception\n");
		else
			fprintf(stderr, "terminate called without an active exception\n");
		abort();
	}
}
