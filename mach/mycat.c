#include <stdio.h>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "need file name\n");
    return 0;
  }

  FILE* fp = fopen(argv[1], "rb");
  char buf[4096];
  size_t len;
  while (1) {
    len = fread(buf, 1, 4096, fp);
    fwrite(buf, 1, len, stdout);
    if (len != 4096) break;
  }
  fclose(fp);
  return 0;
}
