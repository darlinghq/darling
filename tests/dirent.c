#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int main() {
  DIR* dir = opendir("/tmp");
  if (!dir)
    abort();
  int found = 0;
  struct dirent* ent;
  while ((ent = readdir(dir)) != NULL) {
    printf("%s\n", ent->d_name);
    if (!strcmp(ent->d_name, "."))
      found = 1;
  }
  closedir(dir);
  if (!found)
    abort();
  return 0;
}
