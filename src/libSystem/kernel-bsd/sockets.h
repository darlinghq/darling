#ifndef BSD_SOCKETS_H
#define BSD_SOCKETS_H

#define DARWIN_SO_DEBUG 0x1
#define DARWIN_SO_ACCEPTCONN 0x2
#define DARWIN_SO_REUSEADDR 0x4
#define DARWIN_SO_KEEPALIVE 0x8
#define DARWIN_SO_DONTROUTE 0x10
#define DARWIN_SO_BROADCAST 0x20
#define DARWIN_SO_LINGER_SEC 0x1080
#define DARWIN_SO_LINGER 0x80
#define DARWIN_SO_REUSEPORT 0x0200
#define DARWIN_SO_TIMESTAMP 0x0400
#define DARWIN_SO_OOBINLINE 0x0100
#define DARWIN_SO_SNDBUF 0x1001
#define DARWIN_SO_RCVBUF 0x1002
#define DARWIN_SO_RCVLOWAT 0x1004
#define DARWIN_SO_SNDLOWAT 0x1003
#define DARWIN_SO_RCVTIMEO 0x1006
#define DARWIN_SO_SNDTIMEO 0x1005
#define DARWIN_SO_ERROR 0x1007
#define DARWIN_SO_TYPE 0x1008

extern "C"
{

int __darwin_select(int fd, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);
ssize_t __darwin_sendto(int socket, const void *buffer, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len);
ssize_t __darwin_recvfrom(int socket, void * buffer, size_t length, int flags, struct sockaddr * address, socklen_t * address_len);

int __darwin_getsockopt(int socket, int level, int option_name, void * option_value, socklen_t * option_len);

int __darwin_setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);

}

#endif
