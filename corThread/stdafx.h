#ifndef __STD_AFX_INCLUDE_H__
#define __STD_AFX_INCLUDE_H__

#include "define_win.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <setjmp.h>
#include <signal.h>

#if defined(SYS_WIN)
# if(_MSC_VER >= 1300)
#  include <winsock2.h>
#  include <mswsock.h>
# endif
# include <ws2tcpip.h> /* for getaddrinfo */
# include <process.h>
# include <stdint.h>

#define STRDUP _strdup
#define GETPID _getpid
#endif

#endif

struct SOCK_ADDR {
	union {
		struct sockaddr_storage ss;
#ifdef AF_INET6
		struct sockaddr_in6 in6;
#endif
		struct sockaddr_in in;

		struct sockaddr sa;
	} sa;
};

