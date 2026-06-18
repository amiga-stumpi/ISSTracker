#ifndef AMITCP13_BSDSOCKET_H
#define AMITCP13_BSDSOCKET_H
#include <exec/types.h>
#include "socket_limits.h"
#define AF_INET 2
#define SOCK_STREAM 1
#define IPPROTO_TCP 6
#define FIONBIO 0x8004667eUL
#define SOL_SOCKET 0xffff
#define SO_ERROR 0x1007
#define EINPROGRESS 36
#define EWOULDBLOCK 35
#define EAGAIN EWOULDBLOCK
#define ECONNREFUSED 61
#define ETIMEDOUT 60
struct in_addr { ULONG s_addr; };
struct sockaddr { UWORD sa_family; char sa_data[14]; };
struct sockaddr_in { UWORD sin_family; UWORD sin_port; struct in_addr sin_addr; UBYTE sin_zero[8]; };
typedef LONG socklen_t;
struct hostent { char *h_name; char **h_aliases; LONG h_addrtype; LONG h_length; char **h_addr_list; };
#define h_addr h_addr_list[0]
#endif
