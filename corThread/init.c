#include "stdafx.h"
#include "init.h"
#include "pthread_patch.h"

static unsigned long var_main_tid = (unsigned long) -1;

void lib_init(void)
{
	static int __have_inited = 0;

	if (__have_inited)
		return;
	__have_inited = 1;
	var_main_tid = __pthread_self();
}
unsigned long main_thread_self(void)
{
	return var_main_tid;
}