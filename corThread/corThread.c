#include "stdafx.h"
#include "libcorthread.h"
#include "general.h"

#include "corthread.h"

#define	MAX_CACHE	1000

typedef struct THREAD {
	RING       ready;		
	RING       dead;		
	CT_CORTHREAD **corthreads;
	unsigned   size;
	unsigned   slot;
	int        exitcode;
	CT_CORTHREAD *running;
	CT_CORTHREAD *original;
	int        errnum;
	unsigned   idgen;
	int        count;
	size_t     switched;
	int        nlocal;
} THREAD;

static THREAD          *__main_corthread = NULL;
static __thread THREAD *__thread_corthread = NULL;
static __thread int __scheduled = 0;
static __thread int __schedule_auto = 0;
__thread int var_hook_sys_api = 0;

static pthread_key_t __corthread_key;

void ct_corthread_hook_api(int onoff)
{
	var_hook_sys_api = onoff;
}

int ct_corthread_scheduled(void)
{
	return __scheduled;
}

static void thread_free(void *ctx)
{
	THREAD *tf = (THREAD *)ctx;

	if (__thread_corthread == NULL) {
		return;
	}

	if (tf->corthreads) {
		free(tf->corthreads);
	}

	tf->original->free_fn(tf->original);
	free(tf);

	if (__main_corthread == __thread_corthread) {
		__main_corthread = NULL;
	}
	__thread_corthread = NULL;
}

static void corthread_schedule_main_free(void)
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

static void corthread_check(void)
{
	if (__thread_corthread != NULL) {
		return;
	}

	if (pthread_once(&__once_control, thread_init) != 0) {
		msg_fatal("%s(%d), %s: pthread_once error %s",
			__FILE__, __LINE__, __FUNCTION__, last_serror());
	}

	__thread_corthread = (THREAD *)calloc(1, sizeof(THREAD));

	__thread_corthread->original = corthread_win_origin();
	__thread_corthread->corthreads = NULL;
	__thread_corthread->size = 0;
	__thread_corthread->slot = 0;
	__thread_corthread->idgen = 0;
	__thread_corthread->count = 0;
	__thread_corthread->nlocal = 0;

	ring_init(&__thread_corthread->ready);
	ring_init(&__thread_corthread->dead);

	if (__pthread_self() == main_thread_self()) {
		__main_corthread = __thread_corthread;
		atexit(corthread_schedule_main_free);
	}
	else if (pthread_setspecific(__corthread_key, __thread_corthread) != 0) {
		msg_fatal("pthread_setspecific error!");
	}
}


void ct_corthread_set_errno(CT_CORTHREAD *corthread, int errnum)
{
	if (corthread == NULL) {
		corthread = ct_corthread_running();
	}
	if (corthread) {
		corthread->errnum = errnum;
	}
}

int ct_corthread_errno(CT_CORTHREAD *corthread)
{
	if (corthread == NULL) {
		corthread = ct_corthread_running();
	}
	return corthread ? corthread->errnum : 0;
}

void corthread_save_errno(int errnum)
{
	CT_CORTHREAD *curr;

	if (__thread_corthread == NULL) {
		corthread_check();
	}

	if ((curr = __thread_corthread->running) == NULL) {
		curr = __thread_corthread->original;
	}

	if (curr->flag & CORTHREAD_F_SAVE_ERRNO) {
	
		return;
	}

	ct_corthread_set_errno(curr, errnum);
}

static void corthread_kick(int max)
{
	RING *head;
	CT_CORTHREAD *corthread;

	while (max > 0) {
		head = ring_pop_head(&__thread_corthread->dead);
		if (head == NULL) {
			break;
		}
		corthread = RING_TO_APPL(head, CT_CORTHREAD, me);
		corthread_free(corthread);
		max--;
	}
}

static void corthread_swap(CT_CORTHREAD *from, CT_CORTHREAD *to)
{
	if (from->status == CORTHREAD_STATUS_EXITING) {
		size_t slot = from->slot;
		int n = ring_size(&__thread_corthread->dead);

		if (n > MAX_CACHE) {
			n -= MAX_CACHE;
			corthread_kick(n);
		}

		if (!from->sys) {
			__thread_corthread->count--;
		}

		__thread_corthread->corthreads[slot] =
			__thread_corthread->corthreads[--__thread_corthread->slot];
		__thread_corthread->corthreads[slot]->slot = slot;

		ring_prepend(&__thread_corthread->dead, &from->me);
	}

	from->swap_fn(from, to);
}


CT_CORTHREAD *ct_corthread_running(void)
{
	corthread_check();
	return __thread_corthread->running;
}

void ct_corthread_kill(CT_CORTHREAD *corthread)
{
	ct_corthread_signal(corthread, SIGTERM);
}

int ct_corthread_killed(CT_CORTHREAD *corthread)
{
	if (!corthread) {
		corthread = ct_corthread_running();
	}
	return corthread && (corthread->flag & CORTHREAD_F_KILLED);
}

void ct_corthread_signal(CT_CORTHREAD *corthread, int signum)
{
	CT_CORTHREAD *curr = __thread_corthread->running;

	if (corthread == NULL) {
		msg_error("%s(%d), %s: corthread NULL",
			__FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (curr == NULL) {
		msg_error("%s(%d), %s: current corthread NULL",
			__FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (signum == SIGTERM) {

		corthread->errnum = ECANCELED;
		corthread->flag |= CORTHREAD_F_KILLED;
	}

	corthread->signum = signum;

	if (corthread == curr) { 
		return;
	}
	ring_detach(&curr->me);
	ring_detach(&corthread->me);

	curr->status = CORTHREAD_STATUS_READY;
	ring_append(&__thread_corthread->ready, &curr->me);

	corthread->status = CORTHREAD_STATUS_READY;
	ring_append(&__thread_corthread->ready, &corthread->me);

	ct_corthread_switch();
}

int ct_corthread_signum(CT_CORTHREAD *corthread)
{
	if (corthread) {
		corthread = ct_corthread_running();
	}
	return corthread ? corthread->signum : 0;
}

void corthread_exit(int exit_code)
{
	corthread_check();

	__thread_corthread->exitcode = exit_code;
	__thread_corthread->running->status = CORTHREAD_STATUS_EXITING;

	ct_corthread_switch();
}

void ct_corthread_ready(CT_CORTHREAD *corthread)
{
	if (corthread->status != CORTHREAD_STATUS_EXITING) {
		corthread->status = CORTHREAD_STATUS_READY;
		ring_prepend(&__thread_corthread->ready, &corthread->me);
	}
}

int ct_corthread_yield(void)
{
	size_t  n;

	if (ring_size(&__thread_corthread->ready) == 0) {
		return 0;
	}

	n = __thread_corthread->switched;
	ct_corthread_ready(__thread_corthread->running);
	ct_corthread_switch();

	return abs(__thread_corthread->switched - n - 1);
}

int ct_corthread_ndead(void)
{
	if (__thread_corthread == NULL) {
		return 0;
	}
	return ring_size(&__thread_corthread->dead);
}

void corthread_free(CT_CORTHREAD *corthread)
{
	corthread->free_fn(corthread);
}

static void corthread_start(CT_CORTHREAD *corthread)
{
	int i;

	corthread->fn(corthread, corthread->arg);

	for (i = 0; i < corthread->nlocal; i++) {
		if (corthread->locals[i] == NULL) {
			continue;
		}
		if (corthread->locals[i]->free_fn) {
			corthread->locals[i]->free_fn(corthread->locals[i]->ctx);
		}
		free(corthread->locals[i]);
	}

	if (corthread->locals) {
		free(corthread->locals);
		corthread->locals = NULL;
		corthread->nlocal = 0;
	}

	corthread_exit(0);
}

static CT_CORTHREAD *corthread_alloc(void(*fn)(CT_CORTHREAD *, void *),
	void *arg, size_t size)
{
	CT_CORTHREAD *corthread = NULL;
	RING *head;

	corthread_check();

#define	APPL RING_TO_APPL

	head = ring_pop_head(&__thread_corthread->dead);
	if (head == NULL) {

		corthread = corthread_win_alloc(corthread_start, size);

	}
	else {
		corthread = APPL(head, CT_CORTHREAD, me);
	}

	__thread_corthread->idgen++;
	if (__thread_corthread->idgen == 0) { 
		__thread_corthread->idgen++;
	}

	corthread->id = __thread_corthread->idgen;
	corthread->errnum = 0;
	corthread->signum = 0;
	corthread->fn = fn;
	corthread->arg = arg;
	corthread->flag = 0;
	corthread->status = CORTHREAD_STATUS_READY;

	ring_init(&corthread->holding);
	corthread->init_fn(corthread, size);

	return corthread;
}

void ct_corthread_schedule_init(int on)
{
	__schedule_auto = on;
}

CT_CORTHREAD *ct_corthread_create(void(*fn)(CT_CORTHREAD *, void *),void *arg, size_t size)
{
	CT_CORTHREAD *corthread = corthread_alloc(fn, arg, size);

	__thread_corthread->count++;

	if (__thread_corthread->slot >= __thread_corthread->size) {
		__thread_corthread->size += 128;
		__thread_corthread->corthreads = (CT_CORTHREAD **)realloc(
			__thread_corthread->corthreads,
			__thread_corthread->size * sizeof(CT_CORTHREAD *));
	}

	corthread->slot = __thread_corthread->slot;
	__thread_corthread->corthreads[__thread_corthread->slot++] = corthread;

	ct_corthread_ready(corthread);
	if (__schedule_auto && !ct_corthread_scheduled()) {
		ct_corthread_schedule();
	}
	return corthread;
}

unsigned int ct_corthread_id(const CT_CORTHREAD *corthread)
{
	return corthread ? corthread->id : 0;
}

unsigned int ct_corthread_self(void)
{
	CT_CORTHREAD *curr = ct_corthread_running();
	return ct_corthread_id(curr);
}

int ct_corthread_status(const CT_CORTHREAD *corthread)
{
	if (corthread == NULL) {
		corthread = ct_corthread_running();
	}
	return corthread ? corthread->status : 0;
}

void ct_corthread_schedule_set_event(int event_mode)
{
	event_set(event_mode);
}

void ct_corthread_schedule_with(int event_mode)
{
	ct_corthread_schedule_set_event(event_mode);
	ct_corthread_schedule();
}

void ct_corthread_schedule(void)
{
	CT_CORTHREAD *corthread;
	RING *head;

	if (__scheduled) {
		return;
	}

	corthread_check();
	ct_corthread_hook_api(1);
	__scheduled = 1;

	for (;;) {
		head = ring_pop_head(&__thread_corthread->ready);
		if (head == NULL) {
			msg_info("thread-%lu: NO CORTHREAD NOW", __pthread_self());
			break;
		}

		corthread = RING_TO_APPL(head, CT_CORTHREAD, me);
		corthread->status = CORTHREAD_STATUS_READY;

		__thread_corthread->running = corthread;
		__thread_corthread->switched++;

		corthread_swap(__thread_corthread->original, corthread);
		__thread_corthread->running = NULL;
	}

	while ((head = ring_pop_head(&__thread_corthread->dead)) != NULL) {
		corthread = RING_TO_APPL(head, CT_CORTHREAD, me);
		corthread_free(corthread);
	}

	ct_corthread_hook_api(0);
	__scheduled = 0;
}

void corthread_system(void)
{
	if (!__thread_corthread->running->sys) {
		__thread_corthread->running->sys = 1;
		__thread_corthread->count--;
	}
}

void corthread_count_inc(void)
{
	__thread_corthread->count++;
}

void corthread_count_dec(void)
{
	__thread_corthread->count--;
}

void ct_corthread_switch(void)
{
	CT_CORTHREAD *corthread, *current = __thread_corthread->running;
	RING *head;

#ifdef _DEBUG
	assert(current);
#endif

	head = ring_pop_head(&__thread_corthread->ready);
	if (head == NULL) {
		msg_info("thread-%lu: NO CORTHREAD in ready", __pthread_self());
		corthread_swap(current, __thread_corthread->original);
		return;
	}

	corthread = RING_TO_APPL(head, CT_CORTHREAD, me);

	__thread_corthread->running = corthread;
	__thread_corthread->switched++;

	corthread_swap(current, __thread_corthread->running);
}

int ct_corthread_set_specific(int *key, void *ctx, void(*free_fn)(void *))
{
	CORTHREAD_LOCAL *local;
	CT_CORTHREAD *curr;

	if (key == NULL) {
		msg_error("%s(%d), %s: key NULL",
			__FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	if (__thread_corthread == NULL) {
		msg_error("%s(%d), %s: __thread_corthread: NULL",
			__FILE__, __LINE__, __FUNCTION__);
		return -1;
	}
	else if (__thread_corthread->running == NULL) {
		msg_error("%s(%d), %s: running: NULL",
			__FILE__, __LINE__, __FUNCTION__);
		return -1;
	}
	else
		curr = __thread_corthread->running;

	if (*key <= 0) {
		*key = ++__thread_corthread->nlocal;
	}
	else if (*key > __thread_corthread->nlocal) {
		msg_error("%s(%d), %s: invalid key: %d > nlocal: %d",
			__FILE__, __LINE__, __FUNCTION__,
			*key, __thread_corthread->nlocal);
		return -1;
	}

	if (curr->nlocal < __thread_corthread->nlocal) {
		int i, n = curr->nlocal;
		curr->nlocal = __thread_corthread->nlocal;
		curr->locals = (CORTHREAD_LOCAL **)realloc(curr->locals,
			curr->nlocal * sizeof(CORTHREAD_LOCAL*));
		for (i = n; i < curr->nlocal; i++)
			curr->locals[i] = NULL;
	}

	local = (CORTHREAD_LOCAL *)calloc(1, sizeof(CORTHREAD_LOCAL));
	local->ctx = ctx;
	local->free_fn = free_fn;
	curr->locals[*key - 1] = local;

	return *key;
}

void *ct_corthread_get_specific(int key)
{
	CORTHREAD_LOCAL *local;
	CT_CORTHREAD *curr;

	if (key <= 0) {
		return NULL;
	}

	if (__thread_corthread == NULL) {
		msg_error("%s(%d), %s: __thread_corthread NULL",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}
	else if (__thread_corthread->running == NULL) {
		msg_error("%s(%d), %s: running corthread NULL",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}
	else
		curr = __thread_corthread->running;

	if (key > curr->nlocal) {
		return NULL;
	}

	local = curr->locals[key - 1];

	return local ? local->ctx : NULL;
}

int ct_corthread_last_error(void)
{
	int   error;
	error = WSAGetLastError();
	WSASetLastError(error);
	return error;
}

void ct_corthread_set_error(int errnum)
{
	WSASetLastError(errnum);
	errno = errnum;
}
