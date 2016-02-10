/* srm */
/* Copyright (c) 2000 Matthew D. Gauthier
 * Portions copyright (c) 2007 Apple Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the contributors shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization.
 */

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <TargetConditionals.h>

#include "removefile.h"
#include "removefile_priv.h"

static int empty_directory(const char *path) {
  DIR *dp;
  struct dirent *de;
  
  dp = opendir(path);
  if (dp == NULL) {
    return -1;
  }
  while ((de = readdir(dp)) != NULL) {
    if (de->d_namlen < 3 && (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))) {
      continue;
    }
    (void)closedir(dp);
    return -1;
  }
  (void)closedir(dp);
  return 0;
}

int
__removefile_rename_unlink(const char *path, removefile_state_t state) {
  char *p, c;
  struct stat statbuf;

  size_t new_name_size = strlen(path) + 15;
  char new_name[new_name_size];
  int i = 0;
  
  strlcpy(new_name, path, new_name_size);

  if ( (p = strrchr(new_name, '/')) != NULL ) {
    p++;
    *p = '\0';
  } else {
    p = new_name;
  }

  do {
    i = 0;

    while (i < 14) {
      c = __removefile_random_char(state);
      if (isalnum((int) c)) {
	p[i] = c;
	i++;
      }
    }
    p[i] = '\0';
  } while (lstat(new_name, &statbuf) == 0);

  if (lstat(path, &statbuf) == -1)
    return -1;

  if (S_ISDIR(statbuf.st_mode) && (empty_directory(path) == -1)) {
      /* Directory isn't empty (e.g. because it contains an immutable file).
         Attempting to remove it will fail, so avoid renaming it. */
	errno = ENOTEMPTY;
    return -1;
  }

  if (rename(path, new_name) == -1)
    return -1;

  if (lstat(new_name, &statbuf) == -1) {
	errno = ENOENT;
	return -1;
  }

  if (S_ISDIR(statbuf.st_mode))
    return rmdir(new_name);

  return unlink(new_name);
}
