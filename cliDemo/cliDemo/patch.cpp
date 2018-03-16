#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <winsock2.h>
#include "libcorThread.h"
#include "patch.h"

void socket_init(void)
{
	WORD version = 0;
	WSADATA data;

	FillMemory(&data, sizeof(WSADATA), 0);
	version = MAKEWORD(2, 0);

	if (WSAStartup(version, &data) != 0) {
		abort();
	}
}

void socket_end(void)
{
	WSACleanup();
}

void socket_close(SOCKET fd)
{
	ct_corthread_close(fd);
}

SOCKET socket_listen(const char *ip, int port)
{
	SOCKET fd;
	int on;
	struct sockaddr_in sa;

	memset(&sa, 0, sizeof(sa));
	sa.sin_family      = AF_INET;
	sa.sin_port        = htons(port);
	sa.sin_addr.s_addr = inet_addr(ip);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET) {
		printf("create socket error, %s\r\n", strerror(errno));
		getchar();
		exit (1);
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(on))) {
		printf("setsockopt error %s\r\n", strerror(errno));
		exit (1);
	}

	if (bind(fd, (struct sockaddr *) &sa, sizeof(struct sockaddr)) < 0) {
		printf("bind error %s\r\n", strerror(errno));
		getchar();
		exit (1);
	}

	if (ct_corthread_listen(fd, 128) < 0) {
		printf("listen error %s\r\n", strerror(errno));
		getchar();
		exit (1);
	}

	return fd;
}

SOCKET socket_accept(SOCKET fd)
{
	SOCKET cfd;
	struct sockaddr_in sa;
	int len = sizeof(sa);
	
	cfd = ct_corthread_accept(fd, (struct sockaddr *)& sa, (socklen_t *)& len);
	return cfd;
}

SOCKET socket_connect(const char *ip, int port)
{
	SOCKET  fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sa;
	socklen_t len = (socklen_t) sizeof(sa);

	if (fd == INVALID_SOCKET) {
		printf("create socket error %s\r\n", strerror(errno));
		return INVALID_SOCKET;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port   = htons(port);
	sa.sin_addr.s_addr = inet_addr(ip);

	if (ct_corthread_connect(fd, (const struct sockaddr *) &sa, len) < 0) {
		ct_corthread_close(fd);
		printf("%s: connect %s:%d erorr %s\r\n",
			__FUNCTION__, ip, port, strerror(errno));
		return INVALID_SOCKET;
	}
	return fd;
}
