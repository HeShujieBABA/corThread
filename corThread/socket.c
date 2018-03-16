#include "stdafx.h"
#include "general.h"

#include "ct_event.h"
#include "corthread.h"

#ifdef SYS_WIN
typedef socket_t(WINAPI *socket_fn)(int, int, int);
typedef int (WINAPI *listen_fn)(socket_t, int);
typedef socket_t(WINAPI *accept_fn)(socket_t, struct sockaddr *, socklen_t *);
typedef int (WINAPI *connect_fn)(socket_t, const struct sockaddr *, socklen_t);
#endif

static socket_fn  __sys_socket = NULL;
static listen_fn  __sys_listen = NULL;
static accept_fn  __sys_accept = NULL;
static connect_fn __sys_connect = NULL;

static void hook_api(void)
{
	__sys_socket = socket;
	__sys_listen = listen;
	__sys_accept = accept;
	__sys_connect = connect;

}

static pthread_once_t __once_control = PTHREAD_ONCE_INIT;

static void hook_init(void)
{
	if (pthread_once(&__once_control, hook_api) != 0) {
		abort();
	}
}

socket_t ct_corthread_socket(int domain, int type, int protocol)
{
	socket_t sockfd;

	if (__sys_socket == NULL) {
		hook_init();
	}

	if (__sys_socket == NULL) {
		return -1;
	}

	sockfd = __sys_socket(domain, type, protocol);

	if (!var_hook_sys_api) {
		return sockfd;
	}

	if (sockfd != INVALID_SOCKET) {
		non_blocking(sockfd, NON_BLOCKING);
	}
	else {
		corthread_save_errno(ct_corthread_last_error());
	}

	return sockfd;
}

int ct_corthread_listen(socket_t sockfd, int backlog)
{
	if (__sys_listen == NULL) {
		hook_init();
	}

	if (!var_hook_sys_api) {
		return __sys_listen ? __sys_listen(sockfd, backlog) : -1;
	}

	non_blocking(sockfd, NON_BLOCKING);
	if (__sys_listen(sockfd, backlog) == 0) {
		return 0;
	}

	corthread_save_errno(ct_corthread_last_error());
	return -1;
}

#if CORTHREAD_EAGAIN == CORTHREAD_EWOULDBLOCK
# define error_again(x) ((x) == CORTHREAD_EAGAIN)
#endif

#define FAST_ACCEPT

socket_t WINAPI ct_corthread_accept(socket_t sockfd, struct sockaddr *addr,
	socklen_t *addrlen)
{
	FILE_EVENT *fe;
	socket_t clifd;
	int  err;

	if (sockfd == INVALID_SOCKET) {
		msg_error("%s: invalid sockfd %d", __FUNCTION__, sockfd);
		return -1;
	}

	if (__sys_accept == NULL) {
		hook_init();
	}

	if (!var_hook_sys_api) {
		return __sys_accept ?
			__sys_accept(sockfd, addr, addrlen) : INVALID_SOCKET;
	}

#ifdef	FAST_ACCEPT

	non_blocking(sockfd, NON_BLOCKING);

	clifd = __sys_accept(sockfd, addr, addrlen);
	if (clifd != INVALID_SOCKET) {
		non_blocking(clifd, NON_BLOCKING);
		tcp_nodelay(clifd, 1);
		return clifd;
	}

	err = ct_corthread_last_error();
#if CORTHREAD_EAGAIN == CORTHREAD_EWOULDBLOCK
	if (err != CORTHREAD_EAGAIN) {
#endif
		return INVALID_SOCKET;
	}

	fe = corthread_file_open(sockfd);

	while (1) {
		corthread_wait_read(fe);

		if (ct_corthread_killed(fe->corthread)) {
			msg_info("%s(%d), %s: corthread-%u was killed", __FILE__,
				__LINE__, __FUNCTION__, ct_corthread_id(fe->corthread));
			return INVALID_SOCKET;
		}
		clifd = __sys_accept(sockfd, addr, addrlen);

		if (clifd != INVALID_SOCKET) {
			non_blocking(clifd, NON_BLOCKING);
			tcp_nodelay(clifd, 1);
			return clifd;
		}

		err = ct_corthread_last_error();
		corthread_save_errno(err);

		if (!error_again(err)) {
			return INVALID_SOCKET;
		}
	}
}

int WINAPI ct_corthread_connect(socket_t sockfd, const struct sockaddr *addr,socklen_t addrlen)
{
	int err, ret;
	socklen_t len;
	FILE_EVENT *fe;
	time_t begin, end;

	if (__sys_connect == NULL)
		hook_init();

	if (!var_hook_sys_api) {
		return __sys_connect ? __sys_connect(sockfd, addr, addrlen) : -1;
	}

	non_blocking(sockfd, NON_BLOCKING);

	ret = __sys_connect(sockfd, addr, addrlen);
	if (ret >= 0) {
		tcp_nodelay(sockfd, 1);
		return ret;
	}

	err = ct_corthread_last_error();
	corthread_save_errno(err);

#if CORTHREAD_EAGAIN == CORTHREAD_EWOULDBLOCK
	if (err != CORTHREAD_EINPROGRESS && err != CORTHREAD_EAGAIN) {
#endif
		if (err == CORTHREAD_ECONNREFUSED) {
			msg_error("%s(%d), %s: connect ECONNREFUSED",
				__FILE__, __LINE__, __FUNCTION__);
		}
		else if (err == CORTHREAD_ECONNRESET) {
			msg_error("%s(%d), %s: connect ECONNRESET",
				__FILE__, __LINE__, __FUNCTION__);
		}
		else if (err == CORTHREAD_ENETDOWN) {
			msg_error("%s(%d), %s: connect ENETDOWN",
				__FILE__, __LINE__, __FUNCTION__);
		}
		else if (err == CORTHREAD_ENETUNREACH) {
			msg_error("%s(%d), %s: connect ENETUNREACH",
				__FILE__, __LINE__, __FUNCTION__);
		}
		else if (err == CORTHREAD_EHOSTUNREACH) {
			msg_error("%s(%d), %s: connect EHOSTUNREACH",
				__FILE__, __LINE__, __FUNCTION__);
		}
		else {
			msg_error("%s(%d), %s: connect errno=%d, %s",
				__FILE__, __LINE__, __FUNCTION__, err,
				last_serror());
		}

		return -1;
	}

	fe = corthread_file_open(sockfd);

	fe->status |= STATUS_CONNECTING;

	time(&begin);
	corthread_wait_write(fe);
	time(&end);

	fe->status &= ~STATUS_CONNECTING;

	if (ct_corthread_killed(fe->corthread)) {
		msg_info("%s(%d), %s: corthread-%u was killed, %s, spend %ld",
			__FILE__, __LINE__, __FUNCTION__,
			ct_corthread_id(fe->corthread), last_serror(),
			(long)(end - begin));
		return -1;
	}

	len = sizeof(err);
	ret = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&err, &len);
	if (ret == 0 && err == 0) {
		struct SOCK_ADDR saddr;
		struct sockaddr *sa = (struct sockaddr*) &saddr;
		socklen_t n = sizeof(saddr);

		if (getpeername(sockfd, sa, &n) == 0) {
			return 0;
		}

		corthread_save_errno(ct_corthread_last_error());
		msg_error("%s(%d), %s: getpeername error %s, fd: %d, spend %ld",
			__FILE__, __LINE__, __FUNCTION__, last_serror(),
			sockfd, (long)(end - begin));
		return -1;
	}

	ct_corthread_set_error(err);
	msg_error("%s(%d): getsockopt error: %s, ret: %d, err: %d, spend %ld",
		__FUNCTION__, __LINE__, last_serror(), ret, err,
		(long)(end - begin));

	return -1;
}
#endif