#include "stdafx.h"
#include "general.h"

#include "event_select.h"
#include "ct_event.h"

static __thread int __event_mode = CORTHREAD_EVENT_SELECT;

void event_set(int event_mode)
{
	switch (__event_mode) {
	case CORTHREAD_EVENT_SELECT:
		__event_mode = event_mode;
		break;
	default:
		break;
	}
}

EVENT *event_create(int size)
{
	EVENT *ev = NULL;

	switch (__event_mode) {
	case CORTHREAD_EVENT_SELECT:
		ev = event_select_create(size);
		break;
	default:
		break;
	}

	ring_init(&ev->events);
	ev->timeout = -1;
	ev->setsize = size;
	ev->fdcount = 0;
	ev->maxfd = -1;

	return ev;
}

const char *event_name(EVENT *ev)
{
	return ev->name();
}

int event_handle(EVENT *ev)
{
	return ev->handle(ev);
}

int event_size(EVENT *ev)
{
	return ev->setsize;
}

void event_free(EVENT *ev)
{
	ev->free(ev);
}

static int checkfd(EVENT *ev, FILE_EVENT *fe)
{
	if (getsocktype(fe->fd) >= 0) {
		return 0;
	}
	return ev->checkfd(ev, fe);
}


int event_add_read(EVENT *ev, FILE_EVENT *fe, event_proc *proc)
{
	assert(fe);

	if (fe->type == TYPE_NOSOCK) {
		return 0;
	}

	if (fe->fd >= ev->setsize) {
		msg_error("fd: %d >= setsize: %d", fe->fd, ev->setsize);
		ct_corthread_set_error(ERANGE);
		return 0;
	}

	if (fe->oper & EVENT_DEL_READ) {
		fe->oper &= ~EVENT_DEL_READ;
	}

	if (!(fe->mask & EVENT_READ)) {
		if (fe->type == TYPE_NONE) {
			if (checkfd(ev, fe) == -1) {
				fe->type = TYPE_NOSOCK;
				return 0;
			}
			else {
				fe->type = TYPE_SOCK;
			}
		}

		if (fe->me.parent == &fe->me) {
			ring_prepend(&ev->events, &fe->me);
		}

		fe->oper |= EVENT_ADD_READ;
	}

	fe->r_proc = proc;
	return 1;
}

int event_add_write(EVENT *ev, FILE_EVENT *fe, event_proc *proc)
{
	assert(fe);

	if (fe->type == TYPE_NOSOCK) {
		return 0;
	}

	if (fe->fd >= ev->setsize) {
		msg_error("fd: %d >= setsize: %d", fe->fd, ev->setsize);
		ct_corthread_set_error(ERANGE);
		return 0;
	}

	if (fe->oper & EVENT_DEL_WRITE) {
		fe->oper &= ~EVENT_DEL_WRITE;
	}

	if (!(fe->mask & EVENT_WRITE)) {
		if (fe->type == TYPE_NONE) {
			if (checkfd(ev, fe) == -1) {
				fe->type = TYPE_NOSOCK;
				return 0;
			}
			else {
				fe->type = TYPE_SOCK;
			}
		}

		if (fe->me.parent == &fe->me) {
			ring_prepend(&ev->events, &fe->me);
		}

		fe->oper |= EVENT_ADD_WRITE;
	}

	fe->w_proc = proc;
	return 1;
}

void event_del_read(EVENT *ev, FILE_EVENT *fe)
{
	assert(fe);

	if (fe->oper & EVENT_ADD_READ) {
		fe->oper &= ~EVENT_ADD_READ;
	}

	if (fe->mask & EVENT_READ) {
		if (fe->me.parent == &fe->me) {
			ring_prepend(&ev->events, &fe->me);
		}

		fe->oper |= EVENT_DEL_READ;
	}

	fe->r_proc = NULL;
}

void event_del_write(EVENT *ev, FILE_EVENT *fe)
{
	assert(fe);

	if (fe->oper & EVENT_ADD_WRITE) {
		fe->oper &= ~EVENT_ADD_WRITE;
	}

	if (fe->mask & EVENT_WRITE) {
		if (fe->me.parent == &fe->me) {
			ring_prepend(&ev->events, &fe->me);
		}

		fe->oper |= EVENT_DEL_WRITE;
	}

	fe->w_proc = NULL;
}

void event_close(EVENT *ev, FILE_EVENT *fe)
{
	if (fe->mask & EVENT_READ) {
		ev->del_read(ev, fe);
	}

	if (fe->mask & EVENT_WRITE) {
		ev->del_write(ev, fe);
	}

	if (fe->me.parent != &fe->me) {
		ring_detach(&fe->me);
	}

	if (ev->event_fflush) {
		ev->event_fflush(ev);
	}
}

static void event_prepare(EVENT *ev)
{
	FILE_EVENT *fe;
	RING *next;

	while ((next = ring_first(&ev->events))) {
		fe = ring_to_appl(next, FILE_EVENT, me);

		if (fe->oper & EVENT_DEL_READ) {
			ev->del_read(ev, fe);
		}
		if (fe->oper & EVENT_DEL_WRITE) {
			ev->del_write(ev, fe);
		}
		if (fe->oper & EVENT_ADD_READ) {
			ev->add_read(ev, fe);
		}
		if (fe->oper & EVENT_ADD_WRITE) {
			ev->add_write(ev, fe);
		}

		ring_detach(next);
		fe->oper = 0;
	}

	ring_init(&ev->events);
}

#define TO_APPL	ring_to_appl


int event_process(EVENT *ev, int timeout)
{
	int ret;

	if (ev->timeout < 0) {
		if (timeout < 0) {
			timeout = 100;
		}
	}
	else if (timeout < 0) {
		timeout = ev->timeout;
	}
	else if (timeout > ev->timeout) {
		timeout = ev->timeout;
	}

	if (timeout > 1000 || timeout <= 0) {
		timeout = 100;
	}

	event_prepare(ev);
	ret = ev->event_wait(ev, timeout);

	return ret;
}
