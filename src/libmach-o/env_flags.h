// Copyright 2011 Shinichiro Hamaji. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of  conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials
//      provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY Shinichiro Hamaji ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Shinichiro Hamaji OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

#ifndef ENV_FLAGS_H_
#define ENV_FLAGS_H_

#include <stdlib.h>
#include <string.h>

#define LD_MAC_EnvToBool(envname, dflt)                                 \
  (!getenv(envname) ? (dflt) : memchr("tTyY1\0", getenv(envname)[0], 6) != NULL)

#define LD_MAC_DECLARE_VARIABLE(type, name, tn)                         \
  namespace FLAG__namespace_do_not_use_directly_use_LD_MAC_DECLARE_##tn##_instead { \
  extern type FLAGS_##name;                                             \
  }                                                                     \
  using FLAG__namespace_do_not_use_directly_use_LD_MAC_DECLARE_##tn##_instead::FLAGS_##name

#define LD_MAC_DEFINE_VARIABLE(type, name, value, meaning, tn)            \
  namespace FLAG__namespace_do_not_use_directly_use_LD_MAC_DECLARE_##tn##_instead { \
    type FLAGS_##name(value);                                           \
  }                                                                     \
  using FLAG__namespace_do_not_use_directly_use_LD_MAC_DECLARE_##tn##_instead::FLAGS_##name;

#define DECLARE_bool(name)                      \
  LD_MAC_DECLARE_VARIABLE(bool, name, bool)

#define DEFINE_bool(name, value, meaning)                               \
  LD_MAC_DEFINE_VARIABLE(bool, name, LD_MAC_EnvToBool("LD_MAC_" #name, value), \
                         meaning, bool)

#endif  // ENV_FLAGS_H_
