
#include "stdafx.h"
#include "general.h"

#include "libcorthread.h"
#include "ct_event.h"

void file_event_init(FILE_EVENT *fe, int fd)
{
	ring_init(&fe->me);
	fe->corthread = ct_corthread_running();
	fe->fd = fd;
	fe->id = -1;
	fe->status = STATUS_NONE;
	fe->type = TYPE_NONE;
	fe->oper = 0;
	fe->mask = 0;
	fe->r_proc = NULL;
	fe->w_proc = NULL;
}

FILE_EVENT *file_event_alloc(int fd)
{
	FILE_EVENT *fe = (FILE_EVENT *)calloc(1, sizeof(FILE_EVENT));
	file_event_init(fe, fd);
	return fe;
}

void file_event_free(FILE_EVENT *fe)
{
	free(fe);
}
