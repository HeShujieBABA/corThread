#include "stdafx.h"
#ifndef FD_SETSIZE
#define FD_SETSIZE 10240
#endif
#include "general.h"

#include "ct_event.h"
#include "corthread.h"

#ifdef SYS_WIN
typedef int (WINAPI *select_fn)(int, fd_set *, fd_set *,fd_set *, const struct timeval *);
#endif

static select_fn __sys_select = NULL;


static void hook_api(void)
{
	__sys_select = select;
}

static pthread_once_t __once_control = PTHREAD_ONCE_INIT;

static void hook_init(void)
{
	if (pthread_once(&__once_control, hook_api) != 0) {
		abort();
	}
}

#ifdef SYS_WIN
static struct pollfd *get_pollfd(struct pollfd fds[], int cnt, socket_t fd)
{
	int i;

	for (i = 0; i < cnt; i++) {
		if (fds[i].fd = fd) {
			return &fds[i];
		}
	}

	return NULL;
}

static int set_fdset(struct pollfd fds[], unsigned nfds, unsigned *cnt,fd_set *rset, int oper)
{
	unsigned int i;
	struct pollfd *pfd;

	for (i = 0; i < rset->fd_count; i++) {
		pfd = get_pollfd(fds, *cnt, rset->fd_array[i]);
		if (pfd) {
			pfd->events |= oper;
		}
		else if (*cnt >= nfds) {
			msg_error("%s: overflow, nfds=%d, cnt=%d, fd=%u",
				__FUNCTION__, nfds, *cnt, rset->fd_array[i]);
			return -1;
		}
		else {
			fds[i].events = oper;
			fds[i].fd = rset->fd_array[i];
			fds[i].revents = 0;
			(*cnt)++;
		}
	}
	return 0;
}

static struct pollfd *pfds_create(int *nfds, fd_set *readfds,
	fd_set *writefds, fd_set *exceptfds)
{
	struct pollfd *fds;
	unsigned cnt = 0;

	*nfds = 0;
	if (readfds && (int)readfds->fd_count > *nfds) {
		*nfds = readfds->fd_count;
	}
	if (writefds && (int)writefds->fd_count > *nfds) {
		*nfds = writefds->fd_count;
	}
	if (exceptfds && (int)exceptfds->fd_count > *nfds) {
		*nfds = exceptfds->fd_count;
	}

	fds = (struct pollfd *) calloc(*nfds + 1, sizeof(struct pollfd));
	if (readfds && set_fdset(fds, *nfds, &cnt, readfds, POLLIN) == -1) {
		free(fds);
		return NULL;
	}
	if (writefds && set_fdset(fds, *nfds, &cnt, writefds, POLLOUT) == -1) {
		free(fds);
		return NULL;
	}
	if (exceptfds && set_fdset(fds, *nfds, &cnt, exceptfds, POLLERR) == -1) {
		free(fds);
		return NULL;
	}

	return fds;
}
#endif


int WINAPI ct_corthread_select(int nfds, fd_set *readfds, fd_set *writefds,fd_set *exceptfds, const struct timeval *timeout)
{
	socket_t fd;
	struct pollfd *fds;
	int i, timo, nready = 0;

	if (__sys_select == NULL)
		hook_init();

	if (!var_hook_sys_api)
		return __sys_select ? __sys_select
		(nfds, readfds, writefds, exceptfds, timeout) : -1;

	fds = pfds_create(&nfds, readfds, writefds, exceptfds);
	if (fds == NULL) {
		corthread_save_errno(CORTHREAD_EINVAL);
		return -1;
	}

	if (timeout != NULL)
		timo = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
	else
		timo = -1;

	if (readfds)
		FD_ZERO(readfds);
	if (writefds)
		FD_ZERO(writefds);
	if (exceptfds)
		FD_ZERO(exceptfds);

	for (i = 0; i < nfds ; i++) {

		if ((fd = fds[i].fd) == INVALID_SOCKET) {
			continue;
		}

		if (readfds && (fds[i].revents & POLLIN)) {
			FD_SET(fd, readfds);
			nready++;
		}

		if (writefds && (fds[i].revents & POLLOUT)) {
			FD_SET(fd, writefds);
			nready++;
		}

		if (exceptfds && (fds[i].revents & (POLLERR | POLLHUP))) {
			FD_SET(fd, exceptfds);
			nready++;
		}
	}

	free(fds);
	return nready;
}
