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

#include <string>
#include <vector>

#include "mach-o/loader.h"

using namespace std;

class MachO {
 public:
  struct Rebase {
    uint64_t vmaddr;
    uint8_t type;
  };

  struct Bind {
    uint64_t vmaddr;
    const char* name;
    int64_t addend;
    uint8_t type;
    uint8_t ordinal;
    bool is_weak;
  };

  struct Export {
    string name;
    uint64_t addr;
    uint32_t flag;
  };

  struct Symbol {
    string name;
    uint64_t addr;
  };

  static MachO* read(const char* path, const char* arch,
                     bool need_exports = true);

  virtual ~MachO() {}

  const string& filename() const { return filename_; }

  const vector<segment_command_64*>& segments64() const {
    return segments64_;
  }

  const vector<segment_command*>& segments() const {
    return segments_;
  }

  const vector<const char*>& dylibs() const { return dylibs_; }

  const vector<Rebase*>& rebases() const { return rebases_; }

  const vector<Bind*>& binds() const { return binds_; }

  const vector<Export>& exports() const { return exports_; }

  const vector<Symbol>& symbols() const { return symbols_; }

  const char* base() const { return base_; }

  uint64_t entry() const { return entry_; }

  const vector<uint64_t>& init_funcs() const { return init_funcs_; }

  uint64_t dyld_data() const { return dyld_data_; }

  bool is64() const { return is64_; }

  int fd() const { return fd_; }
  size_t offset() const { return offset_; }

 protected:
  string filename_;
  vector<segment_command_64*> segments64_;
  vector<segment_command*> segments_;
  vector<const char*> dylibs_;
  vector<Rebase*> rebases_;
  vector<Bind*> binds_;
  vector<Export> exports_;
  vector<Symbol> symbols_;
  const char* base_;
  uint64_t entry_;
  vector<uint64_t> init_funcs_;
  uint64_t dyld_data_;
  bool is64_;
  int ptrsize_;
  int fd_;
  size_t offset_;
};

#endif  // MACH_O_H_
