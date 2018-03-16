#include "stdafx.h"
#include "msg.h"
#include "iostuff.h"

int non_blocking(int fd, int on)
{
	unsigned long n = on;
	int flags = 0;

	if (ioctlsocket(fd, FIONBIO, &n) < 0) {
		msg_error("ioctlsocket(fd,FIONBIO) failed");
		return -1;
	}
	return flags;
}

