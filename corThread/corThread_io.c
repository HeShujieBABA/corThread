
#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "general.h"

#include "libcorthread.h"
#include "gettimeofday.h"
#include "ct_event.h"
#include "corthread.h"

typedef struct {
	EVENT     *event;
	size_t     io_count;
	CT_CORTHREAD *ev_corthread;
	RING       ev_timer;
	int        nsleeping;
	int        io_stop;
#ifdef SYS_WIN
	HTABLE      *events;
#endif
} CORTHREAD_TLS;

static CORTHREAD_TLS *__main_corthread = NULL;
static __thread CORTHREAD_TLS *__thread_corthread = NULL;

static void corthread_io_loop(CT_CORTHREAD *corthread, void *ctx);

#define MAXFD		1024
#define STACK_SIZE	819200

socket_t var_maxfd = MAXFD;

void ct_corthread_schedule_stop(void)
{
	corthread_io_check();
	__thread_corthread->io_stop = 1;
}

#define RING_TO_CORTHREAD(r) \
	((CT_CORTHREAD *)((char *)(r)-offsetof(CT_CORTHREAD, me)))

#define FIRST_CORTHREAD(head) \
	(ring_succ(head) != (head) ? RING_TO_CORTHREAD(ring_succ(head)) : 0)

static pthread_key_t __corthread_key;

static void thread_free(void *ctx)
{
	CORTHREAD_TLS *tf = (CORTHREAD_TLS *)ctx;

	if (__thread_corthread == NULL) {
		return;
	}

	if (tf->event) {
		event_free(tf->event);
		tf->event = NULL;
	}

	htable_free(tf->events, NULL);

	free(tf);

	if (__main_corthread == __thread_corthread) {
		__main_corthread = NULL;
	}
	__thread_corthread = NULL;
}

static void corthread_io_main_free(void)
{
	if (__main_corthread) {
		thread_free(__main_corthread);
		if (__thread_corthread == __main_corthread) {
			__thread_corthread = NULL;
		}
		__main_corthread = NULL;
	}
}

static void thread_init(void)
{
	if (pthread_key_create(&__corthread_key, thread_free) != 0) {
		msg_fatal("%s(%d), %s: pthread_key_create error %s",
			__FILE__, __LINE__, __FUNCTION__, last_serror());
	}
}

static pthread_once_t __once_control = PTHREAD_ONCE_INIT;

void corthread_io_check(void)
{
	if (__thread_corthread != NULL) {
		if (__thread_corthread->ev_corthread == NULL) {
			__thread_corthread->ev_corthread = ct_corthread_create(
				corthread_io_loop, __thread_corthread->event,
				STACK_SIZE);
			__thread_corthread->io_count = 0;
			__thread_corthread->nsleeping = 0;
			__thread_corthread->io_stop = 0;
			ring_init(&__thread_corthread->ev_timer);
		}
		return;
	}

	if (pthread_once(&__once_control, thread_init) != 0) {
		msg_fatal("%s(%d), %s: pthread_once error %s",
			__FILE__, __LINE__, __FUNCTION__, last_serror());
	}

	var_maxfd = open_limit(0);
	if (var_maxfd <= 0) {
		var_maxfd = MAXFD;
	}

	__thread_corthread = (CORTHREAD_TLS *)malloc(sizeof(CORTHREAD_TLS));
	__thread_corthread->event = event_create(var_maxfd);
	__thread_corthread->ev_corthread = ct_corthread_create(corthread_io_loop,
		__thread_corthread->event, STACK_SIZE);
	__thread_corthread->io_count = 0;
	__thread_corthread->nsleeping = 0;
	__thread_corthread->io_stop = 0;
	ring_init(&__thread_corthread->ev_timer);

	__thread_corthread->events = htable_create(var_maxfd);

	if (__pthread_self() == main_thread_self()) {
		__main_corthread = __thread_corthread;
		atexit(corthread_io_main_free);
	}
	else if (pthread_setspecific(__corthread_key, __thread_corthread) != 0) {
		msg_fatal("pthread_setspecific error!");
	}
}

void corthread_io_dec(void)
{
	corthread_io_check();
	__thread_corthread->io_count--;
}

void corthread_io_inc(void)
{
	corthread_io_check();
	__thread_corthread->io_count++;
}

EVENT *corthread_io_event(void)
{
	corthread_io_check();
	return __thread_corthread->event;
}

static void corthread_io_loop(CT_CORTHREAD *self corthread_unused, void *ctx)
{
	EVENT *ev = (EVENT *)ctx;
	CT_CORTHREAD *timer;
	long long now, last = 0, left;

	corthread_system();

	for (;;) {
		while (ct_corthread_yield() > 0) {}

		timer = FIRST_CORTHREAD(&__thread_corthread->ev_timer);
		if (timer == NULL) {
			left = -1;
		}
		else {
			SET_TIME(now);
			last = now;
			if (now >= timer->when) {
				left = 0;
			}
			else {
				left = timer->when - now;
			}
		}

		assert(left < INT_MAX);

		event_process(ev, left > 0 ? (int)left + 1 : (int)left);

		if (__thread_corthread->io_stop) {
			break;
		}

		if (timer == NULL) {
			if (ev->fdcount > 0) {
				continue;
			}
			msg_info("%s(%d), tid=%lu: fdcount=0", __FUNCTION__,
				__LINE__, __pthread_self());
			break;
		}

		SET_TIME(now);

		if (now - last < left) {
			continue;
		}

		do {
			ring_detach(&timer->me);

			if (!timer->sys && --__thread_corthread->nsleeping == 0) {
				corthread_count_dec();
			}

			ct_corthread_ready(timer);
			timer = FIRST_CORTHREAD(&__thread_corthread->ev_timer);

		} while (timer != NULL && now >= timer->when);
	}

	if (__thread_corthread->io_count > 0) {
		msg_info("%s(%d), %s: waiting io: %d", __FILE__, __LINE__,
			__FUNCTION__, (int)__thread_corthread->io_count);
	}

	msg_info("%s(%d), tid=%lu: IO corthread exit now",
		__FUNCTION__, __LINE__, __pthread_self());
	__thread_corthread->ev_corthread = NULL;
}

#define CHECK_MIN

void doze(unsigned delay)
{
	struct timeval tv;

	tv.tv_sec = delay / 1000;

	tv.tv_usec = (long)(delay - tv.tv_sec * 1000) * 1000;

	while (select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv) < 0) {
		if (ct_corthread_last_error() != EINTR) {
			msg_fatal("doze: select: %s", last_serror());
		}
	}
}

unsigned int ct_corthread_delay(unsigned int milliseconds)
{
	long long when, now;
	CT_CORTHREAD *corthread;
	RING_ITER iter;
	EVENT *ev;
#ifdef	CHECK_MIN
	long long min = -1;
#endif

	if (!var_hook_sys_api) {
		doze(milliseconds);
		return 0;
	}

	corthread_io_check();

	ev = corthread_io_event();

	SET_TIME(when);
	when += milliseconds;

	ring_foreach_reverse(iter, &__thread_corthread->ev_timer) {
		corthread = ring_to_appl(iter.ptr, CT_CORTHREAD, me);
		if (when >= corthread->when) {
#ifdef	CHECK_MIN
			long long n = when - corthread->when;
			if (min == -1 || n < min) {
				min = n;
			}
#endif
			break;
		}
	}

#ifdef	CHECK_MIN
	if ((min >= 0 && min < ev->timeout) || ev->timeout < 0) {
		ev->timeout = (int)milliseconds;
	}
#endif

	corthread = ct_corthread_running();
	corthread->when = when;
	ring_detach(&corthread->me);

	ring_append(iter.ptr, &corthread->me);

	if (!corthread->sys && __thread_corthread->nsleeping++ == 0) {
		corthread_count_inc();
	}

	ct_corthread_switch();

	if (ring_size(&__thread_corthread->ev_timer) == 0) {
		ev->timeout = -1;
	}
	else {
		ev->timeout = (int)min;
	}

	SET_TIME(now);
	if (now < when) {
		return 0;
	}

	return (unsigned int)(now - when);
}

static void corthread_timer_callback(CT_CORTHREAD *corthread, void *ctx)
{
	long long now, left;

	SET_TIME(now);

	for (;;) {
		left = corthread->when > now ? corthread->when - now : 0;
		if (left == 0) {
			break;
		}

		ct_corthread_delay((unsigned int)left);

		SET_TIME(now);
		if (corthread->when <= now) {
			break;
		}
	}

	corthread->timer_fn(corthread, ctx);
	corthread_exit(0);
}

unsigned int ct_corthread_sleep(unsigned int seconds)
{
	return ct_corthread_delay(seconds * 1000) / 1000;
}

static void read_callback(EVENT *ev, FILE_EVENT *fe)
{
	event_del_read(ev, fe);
	ct_corthread_ready(fe->corthread);
	__thread_corthread->io_count--;
}

void corthread_wait_read(FILE_EVENT *fe)
{
	corthread_io_check();

	fe->corthread = ct_corthread_running();
	
	if (!event_add_read(__thread_corthread->event, fe, read_callback))
		return;
	__thread_corthread->io_count++;
	ct_corthread_switch();
}

static void write_callback(EVENT *ev, FILE_EVENT *fe)
{
	event_del_write(ev, fe);
	ct_corthread_ready(fe->corthread);
	__thread_corthread->io_count--;
}

void corthread_wait_write(FILE_EVENT *fe)
{
	corthread_io_check();

	fe->corthread = ct_corthread_running();
	if (!event_add_write(__thread_corthread->event, fe, write_callback))
		return;
	__thread_corthread->io_count++;
	ct_corthread_switch();
}


static FILE_EVENT *corthread_file_get(socket_t fd)
{
	char key[64];

	corthread_io_check();
	
	_itoa(fd, key, 10); 

	return (FILE_EVENT *)htable_find(__thread_corthread->events, key);
}

static void corthread_file_set(FILE_EVENT *fe)
{
	char key[64];

	_itoa(fe->fd, key, 10);

	htable_enter(__thread_corthread->events, key, fe);
}

FILE_EVENT *corthread_file_open(socket_t fd)
{
	FILE_EVENT *fe = corthread_file_get(fd);

	if (fe == NULL) {
		fe = file_event_alloc(fd);
		corthread_file_set(fe);
	}
	return fe;
}

static int corthread_file_del(FILE_EVENT *fe)
{
	char key[64];

	if (fe->fd == INVALID_SOCKET || fe->fd >= var_maxfd) {
		msg_error("%s(%d): invalid fd=%d",
			__FUNCTION__, __LINE__, fe->fd);
		return -1;
	}

	_itoa(fe->fd, key, 10);

	htable_delete(__thread_corthread->events, key, NULL);
	return 0;
}

int corthread_file_close(socket_t fd, int *closed)
{
	FILE_EVENT *fe;
	EVENT *event;

	*closed = 0;

	corthread_io_check();
	if (fd == INVALID_SOCKET || fd >= var_maxfd) {
		msg_error("%s(%d): invalid fd=%u", __FUNCTION__, __LINE__, fd);
		return -1;
	}

	fe = corthread_file_get(fd);
	if (fe == NULL) {
		return 0;
	}

	event = __thread_corthread->event;
	event_close(event, fe);
	corthread_file_del(fe);

	if (event->close_sock) {
		*closed = event->close_sock(event, fe);
	}

	file_event_free(fe);
	return 1;
}

