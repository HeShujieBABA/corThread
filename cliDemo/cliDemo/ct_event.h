#ifndef EVENT_INCLUDE_H
#define EVENT_INCLUDE_H

#include "define_win.h"
#include "gettimeofday.h"

#include "libcorthread.h"

#define SET_TIME(x) do { \
struct timeval _tv; \
	gettimeofday(&_tv, NULL); \
	(x) = ((long long)_tv.tv_sec) * 1000 + ((long long)_tv.tv_usec) / 1000; \
} while (0)

typedef struct FILE_EVENT   FILE_EVENT;
typedef struct EVENT        EVENT;

typedef int  event_oper(EVENT *ev, FILE_EVENT *fe);
typedef void event_proc(EVENT *ev, FILE_EVENT *fe);


/**
* for each connection fd
*/
struct FILE_EVENT {
	RING       me;
	CT_CORTHREAD *corthread;
	socket_t   fd;
	int id;
	unsigned status;
#define	STATUS_NONE		0
#define	STATUS_CONNECTING	(unsigned) (1 << 0)
#define	STATUS_READABLE		(unsigned) (1 << 1)
#define	STATUS_WRITABLE		(unsigned) (1 << 2)

#define	SET_READABLE(x) ((x)->status |= STATUS_READABLE)
#define	SET_WRITABLE(x)	((x)->status |= STATUS_WRITABLE)
#define	CLR_READABLE(x)	((x)->status &= ~STATUS_READABLE)
#define	CLR_WRITABLE(x)	((x)->status &= ~STATUS_WRITABLE)
#define	IS_READABLE(x)	((x)->status & STATUS_READABLE)
#define	IS_WRITABLE(x)	((x)->status & STATUS_WRITABLE)

	unsigned type;
#define	TYPE_NONE		0
#define	TYPE_SOCK		1
#define	TYPE_NOSOCK		2

	unsigned oper;
#define	EVENT_ADD_READ		(unsigned) (1 << 0)
#define	EVENT_DEL_READ		(unsigned) (1 << 1)
#define	EVENT_ADD_WRITE		(unsigned) (1 << 2)
#define	EVENT_DEL_WRITE		(unsigned) (1 << 3)

	unsigned mask;
#define	EVENT_NONE		0
#define	EVENT_READ		(unsigned) (1 << 0)
#define	EVENT_WRITE		(unsigned) (1 << 1)
#define	EVENT_ERROR		(unsigned) (1 << 2)

	event_proc   *r_proc;
	event_proc   *w_proc;
	char *buf;
	int   size;
	int   len;
};
struct EVENT {
	RING events;
	int  timeout;
	int  fdcount;
	socket_t setsize;
	socket_t maxfd;

	unsigned flag;
#define EVENT_F_IOCP (1 << 0)
#define EVENT_IS_IOCP(x) ((x)->flag & EVENT_F_IOCP)

	const char *(*name)(void);
	long(*handle)(EVENT *);
	void(*free)(EVENT *);

	int(*event_fflush)(EVENT *);
	int(*event_wait)(EVENT *, int);

	event_oper *checkfd;
	event_oper *add_read;
	event_oper *add_write;
	event_oper *del_read;
	event_oper *del_write;
	event_oper *close_sock;
};

/* file_event.c */
void file_event_init(FILE_EVENT *fe, int fd);
FILE_EVENT *file_event_alloc(int fd);
void file_event_free(FILE_EVENT *fe);

/* event.c */
void event_set(int event_mode);
EVENT *event_create(int size);
const char *event_name(EVENT *ev);
int  event_handle(EVENT *ev);
int  event_size(EVENT *ev);
void event_free(EVENT *ev);
void event_close(EVENT *ev, FILE_EVENT *fe);

int  event_add_read(EVENT *ev, FILE_EVENT *fe, event_proc *proc);
int  event_add_write(EVENT *ev, FILE_EVENT *fe, event_proc *proc);
void event_del_read(EVENT *ev, FILE_EVENT *fe);
void event_del_write(EVENT *ev, FILE_EVENT *fe);
int  event_process(EVENT *ev, int left);

#endif
