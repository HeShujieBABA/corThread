#ifndef CORTHREAD_HOOK_INCLUDE_H
#define CORTHREAD_HOOK_INCLUDE_H

#include "corthread_define.h"

#pragma comment(lib,"ws2_32.lib")
#ifdef __cplusplus
extern "C" {
#endif

	CORTHREAD_API socket_t ct_corthread_socket(int domain, int type, int protocol);

	CORTHREAD_API int ct_corthread_listen(socket_t, int backlog);

	CORTHREAD_API int WINAPI ct_corthread_close(socket_t fd);

	CORTHREAD_API socket_t WINAPI ct_corthread_accept(socket_t, struct sockaddr *, socklen_t *);

	CORTHREAD_API int WINAPI ct_corthread_connect(socket_t, const struct sockaddr *, socklen_t);

	CORTHREAD_API int WINAPI ct_corthread_recv(socket_t, char* buf, int len, int flags);

	CORTHREAD_API int WINAPI ct_corthread_recvfrom(socket_t, char* buf, size_t len,int flags, struct sockaddr* src_addr, socklen_t* addrlen);

	CORTHREAD_API int WINAPI ct_corthread_send(socket_t, const char* buf,int len, int flags);

	CORTHREAD_API int WINAPI ct_corthread_sendto(socket_t, const char* buf, size_t len,int flags, const struct sockaddr* dest_addr, socklen_t addrlen);

	CORTHREAD_API int WINAPI ct_corthread_select(int nfds, fd_set *readfds,fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout);


#ifdef __cplusplus
}
#endif

#endif