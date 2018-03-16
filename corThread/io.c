#include "stdafx.h"
#include "general.h"

#include "corthread.h"

typedef int      (WINAPI *close_fn)(socket_t);
typedef ssize_t(WINAPI *recv_fn)(socket_t, char *, int, int);
typedef ssize_t(WINAPI *recvfrom_fn)(socket_t, char *, int, int,
struct  sockaddr *, socklen_t *);
typedef ssize_t(WINAPI *send_fn)(socket_t, const char *, int, int);
typedef ssize_t(WINAPI *sendto_fn)(socket_t, const char *, int, int,
const struct sockaddr *, socklen_t);

static close_fn    __sys_close = NULL;
static recv_fn     __sys_recv = NULL;
static recvfrom_fn __sys_recvfrom = NULL;

static send_fn     __sys_send = NULL;
static sendto_fn   __sys_sendto = NULL;

static void hook_api(void)
{
	__sys_close = closesocket;
	__sys_recv = recv;
	__sys_recvfrom = recvfrom;
	__sys_send = send;
	__sys_sendto = sendto;
}

static pthread_once_t __once_control = PTHREAD_ONCE_INIT;

static void hook_init(void)
{
	if (pthread_once(&__once_control, hook_api) != 0) {
		abort();
	}
}

int WINAPI ct_corthread_close(socket_t fd)
{
	int ret, closed;
	if (fd == INVALID_SOCKET) {
		msg_error("%s: invalid fd: %d", __FUNCTION__, fd);
		return -1;
	}

	if (__sys_close == NULL) {
		hook_init();
	}

	if (!var_hook_sys_api) {
		return __sys_close(fd);
	}


	(void)corthread_file_close(fd, &closed);
	if (closed) {
		return 0;
	}

	ret = __sys_close(fd);
	if (ret == 0) {
		return ret;
	}

	corthread_save_errno(ct_corthread_last_error());
	return ret;
}

#if CORTHREAD_EAGAIN == CORTHREAD_EWOULDBLOCK
# define error_again(x) ((x) == CORTHREAD_EAGAIN)
#endif

static ssize_t corthread_iocp_read(FILE_EVENT *fe, char *buf, int len)
{
	fe->buf = buf;
	fe->size = (int)len;
	fe->len = 0;

	while (1) {
		int err;

		corthread_wait_read(fe);
		if (fe->mask & EVENT_ERROR) {
			return -1;
		}

		err = ct_corthread_last_error();
		corthread_save_errno(err);

		if (ct_corthread_killed(fe->corthread)) {
			msg_info("%s(%d), %s: corthread-%u is existing",
				__FILE__, __LINE__, __FUNCTION__,
				ct_corthread_id(fe->corthread));
			return -1;
		}

		return fe->len;
	}
}

int WINAPI ct_corthread_recv(socket_t sockfd, char *buf, int len, int flags)
{
	FILE_EVENT *fe;

	if (sockfd == INVALID_SOCKET) {
		msg_error("%s: invalid sockfd: %d", __FUNCTION__, sockfd);
		return -1;
	}

	if (__sys_recv == NULL) {
		hook_init();
	}

	if (!var_hook_sys_api) {
		return __sys_recv(sockfd, buf, len, flags);
	}

	fe = corthread_file_open(sockfd);

	if (EVENT_IS_IOCP(corthread_io_event())) {
		return corthread_iocp_read(fe, buf, len);
	}

	while (1) {
		ssize_t ret;
		int err;

		if (IS_READABLE(fe)) {
			CLR_READABLE(fe);
		}
		else {
			corthread_wait_read(fe);
		}

		ret = __sys_recv(sockfd, buf, len, flags);
		if (ret >= 0) {
			return ret;
		}

		err = ct_corthread_last_error();
		corthread_save_errno(err);

		if (ct_corthread_killed(fe->corthread)) {
			msg_info("%s(%d), %s: corthread-%u is existing",
				__FILE__, __LINE__, __FUNCTION__,
				ct_corthread_id(fe->corthread));
			return -1;
		}

		if (!error_again(err)) {
			return -1;
		}
	}
}


int WINAPI ct_corthread_recvfrom(socket_t sockfd, char *buf, size_t len,int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
	FILE_EVENT *fe;

	if (sockfd == INVALID_SOCKET) {
		msg_error("%s: invalid sockfd: %d", __FUNCTION__, sockfd);
		return -1;
	}

	if (__sys_recvfrom == NULL) {
		hook_init();
	}

	if (!var_hook_sys_api) {
		return __sys_recvfrom(sockfd, buf, len, flags,src_addr, addrlen);
	}

	fe = corthread_file_open(sockfd);

	if (EVENT_IS_IOCP(corthread_io_event())) {
		return corthread_iocp_read(fe, buf, len);
	}

	while (1) {
		ssize_t ret;
		int err;

		if (IS_READABLE(fe)) {
			CLR_READABLE(fe);
		}
		else {
			corthread_wait_read(fe);
		}

		ret = __sys_recvfrom(sockfd, buf, len, flags,
			src_addr, addrlen);
		if (ret >= 0) {
			return ret;
		}

		err = ct_corthread_last_error();
		corthread_save_errno(err);

		if (ct_corthread_killed(fe->corthread)) {
			msg_info("%s(%d), %s: corthread-%u is existing",
				__FILE__, __LINE__, __FUNCTION__,
				ct_corthread_id(fe->corthread));
			return -1;
		}

		if (!error_again(err)) {
			return -1;
		}
	}
}

int WINAPI ct_corthread_send(socket_t sockfd, const char *buf,int len, int flags)
{
	if (__sys_send == NULL) {
		hook_init();
	}

	while (1) {
		ssize_t n = __sys_send(sockfd, buf, len, flags);
		FILE_EVENT *fe;
		int err;

		if (!var_hook_sys_api) {
			return n;
		}

		if (n >= 0) {
			return n;
		}

		err = ct_corthread_last_error();
		corthread_save_errno(err);

		if (!error_again(err)) {
			return -1;
		}

		fe = corthread_file_open(sockfd);
		corthread_wait_write(fe);

		if (ct_corthread_killed(fe->corthread)) {
			msg_info("%s(%d), %s: corthread-%u is existing",
				__FILE__, __LINE__, __FUNCTION__,
				ct_corthread_id(fe->corthread));
			return -1;
		}
	}
}


int WINAPI ct_corthread_sendto(socket_t sockfd, const char *buf, size_t len,int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	if (__sys_sendto == NULL) {
		hook_init();
	}

	while (1) {
		ssize_t n = __sys_sendto(sockfd, buf, len, flags,dest_addr, addrlen);
		FILE_EVENT *fe;
		int err;

		if (!var_hook_sys_api) {
			return n;
		}

		if (n >= 0) {
			return n;
		}

		err = ct_corthread_last_error();
		corthread_save_errno(err);

		if (!error_again(err)) {
			return -1;
		}

		fe = corthread_file_open(sockfd);
		corthread_wait_write(fe);

		if (ct_corthread_killed(fe->corthread)) {
			msg_info("%s(%d), %s: corthread-%u is existing",
				__FILE__, __LINE__, __FUNCTION__,
				ct_corthread_id(fe->corthread));
			return -1;
		}
	}
}
