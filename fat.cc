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

#include "fat.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void fixEndian(uint32_t* p, bool be) {
  if (!be) {
    return;
  }

  uint32_t v = *p;
  *p = (v << 24) | ((v << 8) & 0x00ff0000) | ((v >> 8) & 0xff00) | (v >> 24);
}

static string getArchName(uint32_t a) {
  switch (a) {
  case CPU_TYPE_X86:
    return "i386";
  case CPU_TYPE_X86_64:
    return "x86-64";
  case CPU_TYPE_POWERPC:
    return "ppc";
  case CPU_TYPE_POWERPC64:
    return "ppc64";
  default:
    char buf[99];
    sprintf(buf, "??? (%u)", a);
    return buf;
  }
}

bool readFatInfo(int fd, map<string, fat_arch>* fat) {
  lseek(fd, 0, SEEK_SET);

  fat_header header;
  ssize_t len = read(fd, &header, sizeof(header));
  if (len < 0) {
    perror("read");
    exit(1);
  }
  if (len != sizeof(header)) {
    return false;
  }

  bool be = false;
  if (header.magic == FAT_CIGAM) {
    be = true;
  } else if (header.magic != FAT_MAGIC) {
    return false;
  }

  fixEndian(&header.nfat_arch, be);
  for (uint32_t i = 0; i < header.nfat_arch; i++) {
    fat_arch arch;
    len = read(fd, &arch, sizeof(arch));

    fixEndian(&arch.cputype, be);
    fixEndian(&arch.cpusubtype, be);
    fixEndian(&arch.offset, be);
    fixEndian(&arch.size, be);
    fixEndian(&arch.align, be);

    const string& name = getArchName(arch.cputype);
    fat->insert(make_pair(name, arch));
  }

  return true;
}
