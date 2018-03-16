#ifndef CORTHREAD_BASE_INCLUDE_H
#define CORTHREAD_BASE_INCLUDE_H

#include "corthread_define.h"

#ifdef __cplusplus
extern "C" {
#endif

	CORTHREAD_API void ct_corthread_hook_api(int onoff);

	CORTHREAD_API CT_CORTHREAD* ct_corthread_create(void(*fn)(CT_CORTHREAD*, void*),void* arg, size_t size);

	CORTHREAD_API int ct_corthread_ndead(void);

	CORTHREAD_API CT_CORTHREAD* ct_corthread_running(void);

	CORTHREAD_API unsigned int ct_corthread_id(const CT_CORTHREAD* corthread);

	CORTHREAD_API unsigned int ct_corthread_self(void);

	CORTHREAD_API void ct_corthread_set_errno(CT_CORTHREAD* corthread, int errnum);

	CORTHREAD_API int ct_corthread_errno(CT_CORTHREAD* corthread);

	CORTHREAD_API int ct_corthread_status(const CT_CORTHREAD* corthread);

	CORTHREAD_API void ct_corthread_kill(CT_CORTHREAD* corthread);

	CORTHREAD_API int ct_corthread_killed(CT_CORTHREAD* corthread);

	CORTHREAD_API void ct_corthread_signal(CT_CORTHREAD* corthread, int signum);

	CORTHREAD_API int ct_corthread_signum(CT_CORTHREAD* corthread);

	CORTHREAD_API int ct_corthread_yield(void);

	CORTHREAD_API void ct_corthread_ready(CT_CORTHREAD* corthread);

	CORTHREAD_API void ct_corthread_switch(void);

	CORTHREAD_API void ct_corthread_schedule_init(int on);

	CORTHREAD_API void ct_corthread_schedule(void);

#define CORTHREAD_EVENT_SELECT	2	

	CORTHREAD_API void ct_corthread_schedule_with(int event_mode);

	CORTHREAD_API void ct_corthread_schedule_set_event(int event_mode);

	CORTHREAD_API int ct_corthread_scheduled(void);

	CORTHREAD_API void ct_corthread_schedule_stop(void);

	CORTHREAD_API unsigned int ct_corthread_delay(unsigned int milliseconds);

	CORTHREAD_API unsigned int ct_corthread_sleep(unsigned int seconds);

	CORTHREAD_API int ct_corthread_set_specific(int* key, void* ctx, void(*free_fn)(void*));

	CORTHREAD_API void* ct_corthread_get_specific(int key);

	typedef void(*CORTHREAD_MSG_PRE_WRITE_FN)(void *ctx, const char *fmt, va_list ap);

	typedef void(*CORTHREAD_MSG_WRITE_FN) (void *ctx, const char *fmt, va_list ap);

	CORTHREAD_API void ct_corthread_msg_register(CORTHREAD_MSG_WRITE_FN write_fn, void *ctx);

	CORTHREAD_API void ct_corthread_msg_unregister(void);

	CORTHREAD_API void ct_corthread_msg_pre_write(CORTHREAD_MSG_PRE_WRITE_FN pre_write, void *ctx);

	CORTHREAD_API void ct_corthread_msg_stdout_enable(int onoff);

	CORTHREAD_API int ct_corthread_last_error(void);

	CORTHREAD_API const char *ct_corthread_last_serror(void);

	void ct_corthread_set_error(int errnum);

#ifdef __cplusplus
}
#endif

#endif