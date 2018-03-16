#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "libcorThread.h"
#include "patch.h" 

static size_t      __stack_size = 128000;
static const char *__listen_ip = "127.0.0.1";
static int         __listen_port = 8888;

static void fiber_client(CT_CORTHREAD *fb, void *ctx)
{
	SOCKET *pfd = (SOCKET *)ctx;
	char buf[100];

	while (1) {
		memset(buf, 0, sizeof(buf));

		int ret = ct_corthread_recv(*pfd, buf, sizeof(buf), 0);

		if (ret > 0){ 
			printf("%s\n", buf);
		}
		if (ret == 0) {
			break;
		}
		else if (ret < 0) {
			break;
		}

		if (ct_corthread_send(*pfd, buf, ret, 0) < 0) {		
			break;
		}
	}
	socket_close(*pfd);
	free(pfd);
}

static void fiber_accept(CT_CORTHREAD *fb, void *ctx)
{
	const char *addr = (const char *)ctx;
	SOCKET lfd = socket_listen(__listen_ip, __listen_port);
	assert(lfd >= 0);
	for (;;) {
		SOCKET *pfd, cfd = socket_accept(lfd);
		if (cfd == INVALID_SOCKET) {
			printf("accept error %s\r\n", ct_corthread_last_serror());
			break;
		}
		pfd = (SOCKET *)malloc(sizeof(SOCKET));
		*pfd = cfd;

		ct_corthread_create(fiber_client, pfd, __stack_size);
	}
	socket_close(lfd);
	exit(0);
}

int main(void)
{
	int event_mode = CORTHREAD_EVENT_SELECT;
	socket_init();

	ct_corthread_create(fiber_accept, NULL, __stack_size);

	ct_corthread_schedule_with(event_mode);
	socket_end();
	return 0;
}
