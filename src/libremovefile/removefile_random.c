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

#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include "removefile.h"
#include "removefile_priv.h"

static void
seed_random(removefile_state_t state) {
#ifdef USE_ARC4RANDOM
  arc4random_stir();
#else
  unsigned int rseed;
  struct timeval tv;
  struct timezone tz;

  if (state->urand_file != -1) {
    read(state->urand_file, &rseed, sizeof(rseed));
  } else {
    rseed = rand();
  }
  (void)gettimeofday(&tv, &tz);
  rseed ^= tv.tv_sec + tv.tv_usec + getpid();
  srand(rseed);
#endif
}

void
__removefile_init_random(const unsigned int seed, removefile_state_t state) {
#ifdef USE_ARC4RANDOM
  arc4random_addrandom((unsigned char *)&seed, sizeof(seed));
#else
  struct stat statbuf;

  if (stat("/dev/urandom", &statbuf) == 0 && S_ISCHR(statbuf.st_mode)) {
    state->urand_file = open("/dev/urandom", O_RDONLY);
  } else {
    srand(seed);
  }
  seed_random(state);
#endif
}

char
__removefile_random_char(removefile_state_t state) {
#ifdef USE_ARC4RANDOM
  random_bytes_read += 4;
  return arc4random();
#else
  char buf[4];

  if (state->urand_file != -1) {
    read(state->urand_file, &buf, 1);
    return buf[0];
  }
  return rand();
#endif
}

void
__removefile_randomize_buffer(unsigned char *buffer, size_t length, removefile_state_t state) {
  size_t i;

#ifdef USE_ARC4RANDOM
  u_int32_t *p = (u_int32_t *)buffer;
  u_int32_t mod4length = length - (length % 4);

  for (i = 0; i < mod4length; i += 4) {
    *p++ = arc4random();
  }

  while (i < length) {
    buffer[i++] = arc4random();
  }
  state->random_bytes_read += (mod4length + ((length - mod4length) * 4));
  if (state->random_bytes_read > 512*1024*1024 /* RESEED_BYTES */) {
    state->random_bytes_read = 0;
    seed_random(state);
  }
#else
  if (state->urand_file != -1) {
    read(state->urand_file, buffer, length);
  } else {
    for (i = 0; i < length; i++)
      buffer[i] = rand();
  }
#endif
}
