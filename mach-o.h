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

#ifndef MACH_O_H_
#define MACH_O_H_

#include <stdint.h>

#include <vector>

#include "mach-o/loader.h"

using namespace std;

class MachO {
 public:
  struct Bind {
    uint64_t vmaddr;
    const char* name;
    int64_t addend;
    uint8_t type;
    uint8_t ordinal;
  };

  explicit MachO(const char* filename);
  MachO(int fd, size_t offset, size_t len);

  ~MachO();

  const vector<segment_command_64*>& segments64() const {
    return segments64_;
  }

  const vector<segment_command*>& segments() const {
    return segments_;
  }

  const vector<Bind*> binds() const { return binds_; }

  const char* base() const { return base_; }

  uint64_t entry() const { return entry_; }

  bool is64() const { return is64_; }

  int fd() const { return fd_; }

 private:
  // If len is 0, the size of file will be used as len.
  void init(int fd, size_t offset, size_t len);
  void readBind(const uint8_t* p, const uint8_t* end);

  vector<segment_command_64*> segments64_;
  vector<segment_command*> segments_;
  vector<Bind*> binds_;
  const char* base_;
  uint64_t entry_;
  bool is64_;
  int fd_;
};

#endif  // MACH_O_H_
