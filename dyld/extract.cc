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

#include "fat.h"

using namespace std;

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
  if (bin == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  map<string, fat_arch> archs;
  if (!readFatInfo(fd, &archs)) {
    fprintf(stderr, "Not fat\n");
    exit(1);
  }

  printf("%lu archs:\n", (unsigned long)archs.size());
  for (map<string, fat_arch>::const_iterator iter = archs.begin();
       iter != archs.end();
       ++iter) {
    const fat_arch& arch = iter->second;
    printf("cputype=%d (%s) cpusubtype=%d offset=%d size=%d align=%d\n",
           arch.cputype, iter->first.c_str(), arch.cpusubtype,
           arch.offset, arch.size, arch.align);
  }

  for (int i = 2; i + 1 < argc; i += 2) {
    const char* arch_name = argv[i];
    map<string, fat_arch>::const_iterator found = archs.find(arch_name);
    if (found == archs.end()) {
      printf("unknown arch: %s\n", arch_name);
      continue;
    }

    const fat_arch& arch = found->second;
    FILE* fp = fopen(argv[i+1], "wb");
    if (fwrite(bin + arch.offset, 1, arch.size, fp) != arch.size) {
        printf("failed to fwrite\n");
        exit(1);
    }
    fclose(fp);
  }
}
