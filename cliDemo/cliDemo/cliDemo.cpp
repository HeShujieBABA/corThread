#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "libcorThread.h"
#include "patch.h" 
#include <iostream>
using namespace std;

static const char *__server_ip = "127.0.0.1";
static int         __server_port = 8888;


static void fiber_client(CT_CORTHREAD *fb, void *ctx)
{
	SOCKET cfd = socket_connect(__server_ip, __server_port);
	if (cfd == INVALID_SOCKET) {
		return;
	}

	char buf[100];
	int ret;
	
	while(1){
		memset(buf, 0, sizeof(buf));

		cin >> buf;
		if (ct_corthread_send(cfd, buf, sizeof(buf), 0) <= 0) {

			printf("send error %s", ct_corthread_last_serror());
			break;
		}

		memset(buf, 0, sizeof(buf));
		ret = ct_corthread_recv(cfd, buf, sizeof(buf), 0);	
		if (ret <= 0) {
			break;
		}
		else
		{
			printf("%s\n", buf);
		}
	}
	ct_corthread_close(cfd);

}

int main(void)
{
	int event_mode = CORTHREAD_EVENT_SELECT;
	size_t stack_size = 128000;

	socket_init();	
	ct_corthread_create(fiber_client,NULL,stack_size);

	ct_corthread_schedule_with(event_mode);
	socket_end();

	return 0;
}