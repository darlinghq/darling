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

// A command line tool to extract a Mach-O binary from a fat binary.

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <map>
#include <string>

#include "mach-o/fat.h"

using namespace std;

bool be = false;

static void fixEndian(uint32_t* p) {
  if (!be) {
    return;
  }

  uint32_t v = *p;
  *p = (v << 24) | ((v << 8) & 0x00ff0000) | ((v >> 8) & 0xff00) | (v >> 24);
}

static const char* getArchName(uint32_t a) {
  switch (a) {
  case CPU_TYPE_X86:
    return "x86";
  case CPU_TYPE_X86_64:
    return "x86-64";
  case CPU_TYPE_POWERPC:
    return "ppc";
  case CPU_TYPE_POWERPC64:
    return "ppc64";
  default:
    return "???";
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s fat [arch out]\n", argv[0]);
    exit(1);
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  off_t len = lseek(fd, 0, SEEK_END);
  char* bin = reinterpret_cast<char*>(
    mmap(NULL, len,
         PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0));

  fat_header* header = reinterpret_cast<fat_header*>(bin);

  if (header->magic == FAT_CIGAM) {
    be = true;
  } else if (header->magic != FAT_MAGIC) {
    fprintf(stderr, "Not fat\n");
    exit(1);
  }

  fixEndian(&header->nfat_arch);

  printf("magic=%x nfat_arch=%d\n",
         header->magic, header->nfat_arch);

  map<string, fat_arch*> archs;

  char* fat_ptr = bin + sizeof(fat_header);
  for (uint32_t i = 0; i < header->nfat_arch; i++) {
    fat_arch* arch = reinterpret_cast<fat_arch*>(fat_ptr);

    fixEndian(&arch->cputype);
    fixEndian(&arch->cpusubtype);
    fixEndian(&arch->offset);
    fixEndian(&arch->size);
    fixEndian(&arch->align);

    const char* name = getArchName(arch->cputype);

    printf("cputype=%d (%s) cpusubtype=%d offset=%d size=%d align=%d\n",
           arch->cputype, name, arch->cpusubtype,
           arch->offset, arch->size, arch->align);

    archs.insert(make_pair(name, arch));

    fat_ptr += sizeof(fat_arch);
  }

  for (int i = 2; i + 1 < argc; i += 2) {
    const char* arch_name = argv[i];
    map<string, fat_arch*>::const_iterator found = archs.find(arch_name);
    if (found == archs.end()) {
      printf("unknown arch: %s\n", arch_name);
      continue;
    }

    fat_arch* arch = found->second;
    FILE* fp = fopen(argv[i+1], "wb");
    fwrite(bin + arch->offset, 1, arch->size, fp);
    fclose(fp);
  }
}
