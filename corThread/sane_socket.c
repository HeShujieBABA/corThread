#include "stdafx.h"
#include "libcorthread.h"
#include "sane_socket.h"

#pragma comment(lib,"ws2_32.lib")

int is_listen_socket(socket_t fd)
{
	int val, ret;

	int len = sizeof(val);

	ret = getsockopt(fd, SOL_SOCKET, SO_ACCEPTCONN, (void*)&val, &len);
	if (ret == -1) {
		return 0;
	}
	else if (val) {
		return 1;
	}
	else {
		return 0;
	}
}

int getsocktype(socket_t fd)
{
	struct SOCK_ADDR addr;
	struct sockaddr *sa = (struct sockaddr*) &addr;
	socklen_t len = sizeof(addr);

	if (fd == INVALID_SOCKET) {
		return -1;
	}

	if (getsockname(fd, sa, &len) == -1) {
		return -1;
	}

	if (sa->sa_family == AF_INET || sa->sa_family == AF_INET6) {

		return sa->sa_family;
	}

	return -1;

}