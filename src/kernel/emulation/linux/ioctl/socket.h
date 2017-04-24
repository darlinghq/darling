#ifndef EMU_SOCKET_H
#define EMU_SOCKET_H
#include "ioctl.h"

int handle_socket(int fd, unsigned int cmd, void* arg, int* retval);

#endif

