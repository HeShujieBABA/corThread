#ifndef CORTHREAD_INCLUDE_H
#define CORTHREAD_INCLUDE_H

#include "ct_event.h"

typedef enum {
	CORTHREAD_STATUS_READY,
	CORTHREAD_STATUS_RUNNING,
	CORTHREAD_STATUS_EXITING,
} corthread_status_t;

typedef struct {
	void  *ctx;
	void (*free_fn)(void *);
} CORTHREAD_LOCAL;

typedef struct CORTHREAD_BASE {
#define	FBASE_F_BASE	    (1 << 0)
#define FBASE_F_CORTHREAD	(1 << 1)
	unsigned  flag;
	long long atomic_value;
	int       mutex_in;
	int       mutex_out;
	RING      mutex_waiter;
} CORTHREAD_BASE;

struct CT_CORTHREAD {
	CORTHREAD_BASE     base;
	corthread_status_t status;
	RING           me;
	unsigned       id;
	unsigned       slot;
	long long      when;
	int            errnum;
	int            sys;
	int            signum;
	unsigned int   flag;

	RING           holding;

#define CORTHREAD_F_SAVE_ERRNO	(unsigned) 1 << 0
#define	CORTHREAD_F_KILLED		(unsigned) 1 << 1

	CORTHREAD_LOCAL  **locals;
	int            nlocal;

	void(*init_fn)(CT_CORTHREAD *, size_t);
	void(*free_fn)(CT_CORTHREAD *);
	void(*swap_fn)(CT_CORTHREAD *, CT_CORTHREAD *);
	void(*start_fn)(CT_CORTHREAD *);

	void(*fn)(CT_CORTHREAD *, void *);
	void  *arg;
	void(*timer_fn)(CT_CORTHREAD *, void *);
};

extern __thread int var_hook_sys_api;

void corthread_free(CT_CORTHREAD *corthread);

void corthread_save_errno(int errnum);
void corthread_exit(int exit_code);
void corthread_system(void);
void corthread_count_inc(void);
void corthread_count_dec(void);

extern socket_t var_maxfd;

void corthread_io_check(void);
void corthread_wait_read(FILE_EVENT *fe);
void corthread_wait_write(FILE_EVENT *fe);
void corthread_io_dec(void);
void corthread_io_inc(void);
EVENT *corthread_io_event(void);

FILE_EVENT *corthread_file_open(socket_t fd);
int corthread_file_close(socket_t fd, int *closed);

CT_CORTHREAD *corthread_win_origin(void);
CT_CORTHREAD *corthread_win_alloc(void(*start_fn)(CT_CORTHREAD *), size_t size);

#endif