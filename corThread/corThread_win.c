#include "stdafx.h"
#include "general.h"
#include "corthread.h"

#ifdef SYS_WIN

typedef struct CORTHREAD_WIN {
	CT_CORTHREAD corthread;
	LPVOID context;
} CORTHREAD_WIN;

static void corthread_win_free(CT_CORTHREAD *corthread)
{
	CORTHREAD_WIN *fb = (CORTHREAD_WIN *)corthread;
	DeleteFiber(fb->context);
	free(fb);
}

static void corthread_win_swap(CT_CORTHREAD *from, CT_CORTHREAD *to)
{
	CORTHREAD_WIN *fb_to = (CORTHREAD_WIN *)to;
	SwitchToFiber(fb_to->context);
}

static void WINAPI corthread_win_start(LPVOID ctx)
{
	CORTHREAD_WIN *fb = (CORTHREAD_WIN *)ctx;
	fb->corthread.start_fn(&fb->corthread);
}

static void corthread_win_init(CORTHREAD_WIN *fb, size_t size)
{
	if (fb->context) {
		DeleteFiber(fb->context);
	}
	fb->context = CreateFiberEx(0, size, FIBER_FLAG_FLOAT_SWITCH,
		corthread_win_start, fb);
	if (fb->context == NULL) {
		int e = ct_corthread_last_error();
		msg_fatal("%s: CreateFiberEx error=%s, %d", last_serror(), e);
	}
}

CT_CORTHREAD *corthread_win_alloc(void(*start_fn)(CT_CORTHREAD *), size_t size)
{
	CORTHREAD_WIN *fb = (CORTHREAD_WIN *)calloc(1, sizeof(*fb));

	fb->corthread.init_fn = (void(*)(CT_CORTHREAD*, size_t)) corthread_win_init;
	fb->corthread.free_fn = corthread_win_free;
	fb->corthread.swap_fn = (void(*)(CT_CORTHREAD*, CT_CORTHREAD*)) corthread_win_swap;
	fb->corthread.start_fn = start_fn;
	fb->context = NULL;

	return (CT_CORTHREAD *)fb;
}

CT_CORTHREAD *corthread_win_origin(void)
{
	CORTHREAD_WIN *fb = (CORTHREAD_WIN *)calloc(1, sizeof(*fb));

	fb->context = ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
	fb->corthread.free_fn = corthread_win_free;
	fb->corthread.swap_fn = (void(*)(CT_CORTHREAD*, CT_CORTHREAD*)) corthread_win_swap;
	return (CT_CORTHREAD *)fb;
}

#endif
