#ifndef CORTHREAD_DEFINE_INCLUDE_H
#define CORTHREAD_DEFINE_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined (_WIN64)
# include <winsock2.h>

	typedef long ssize_t;
	typedef SOCKET socket_t;
	typedef int socklen_t;

# define	CORTHREAD_ETIMEDOUT		WSAETIMEDOUT
# define	CORTHREAD_ENOMEM		WSAENOBUFS
# define	CORTHREAD_EINVAL		WSAEINVAL
# define	CORTHREAD_ECONNREFUSED	WSAECONNREFUSED
# define	CORTHREAD_ECONNRESET	WSAECONNRESET
# define	CORTHREAD_EHOSTDOWN		WSAEHOSTDOWN
# define	CORTHREAD_EHOSTUNREACH	WSAEHOSTUNREACH
# define	CORTHREAD_EINTR		WSAEINTR
# define	CORTHREAD_ENETDOWN		WSAENETDOWN
# define	CORTHREAD_ENETUNREACH	WSAENETUNREACH
# define	CORTHREAD_ENOTCONN		WSAENOTCONN
# define	CORTHREAD_EISCONN		WSAEISCONN
# define	CORTHREAD_EWOULDBLOCK	WSAEWOULDBLOCK
# define	CORTHREAD_EAGAIN		CORTHREAD_EWOULDBLOCK	/* xxx */
# define	CORTHREAD_ENOBUFS		WSAENOBUFS
# define	CORTHREAD_ECONNABORTED	WSAECONNABORTED
# define	CORTHREAD_EINPROGRESS	WSAEINPROGRESS

# define CORTHREAD_API

#endif

	/**
	* the corthread struct type definition
	*/
	typedef struct CT_CORTHREAD CT_CORTHREAD;

#ifdef __cplusplus
}
#endif

#endif
