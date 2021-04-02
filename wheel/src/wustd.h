#pragma once

#include <iostream>
#include <thread>

typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;

typedef std::uint8_t  uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

#if defined(__LINUX__)
typedef int SOCKET;
typedef struct sockaddr_in sockaddr_in;
#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
typedef int DISPATCH_HANDLE;
#define INVALID_DISPATCH -1
typedef struct epoll_event EVENT_HANDLE;

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <linux/un.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>  
#include <net/if.h>  
#include <sys/file.h>

#else

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <stdio.h>

typedef HANDLE DISPATCH_HANDLE;
#define INVALID_DISPATCH NULL
typedef OVERLAPPED_ENTRY EVENT_HANDLE;
// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#endif

