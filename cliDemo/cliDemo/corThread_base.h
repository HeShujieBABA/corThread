#ifndef CORTHREAD_BASE_INCLUDE_H
#define CORTHREAD_BASE_INCLUDE_H

#include "corthread_define.h"

#ifdef __cplusplus
extern "C" {
#endif

	/**
	* set flag if the system API should be hooked, default value is 1 internal
	* @param onoff {int} if need to hook the system API
	*/
	CORTHREAD_API void ct_corthread_hook_api(int onoff);

	/**
	* create and start one corthread
	* @param fn {void (*)(CT_CORTHREAD*, void*)} the callback of corthread running
	* @param arg {void*} the second parameter of the callback fn
	* @param size {size_t} the virual memory size of the corthread created
	* @return {CT_CORTHREAD*}
	*/
	CORTHREAD_API CT_CORTHREAD* ct_corthread_create(void(*fn)(CT_CORTHREAD*, void*),
		void* arg, size_t size);

	/**
	* get the corthreads count in deading status
	* @retur {int}
	*/
	CORTHREAD_API int ct_corthread_ndead(void);

	/**
	* create one corthread in background for freeing the dead corthreads, specify the
	* maximum corthreads in every recyling process
	* @param max {size_t} the maximum corthreads to freed in every recyling process
	*/
	CORTHREAD_API void ct_corthread_check_timer(size_t max);

	/**
	* get the current running corthread
	* @retur {CT_CORTHREAD*} if no running corthread NULL will be returned
	*/
	CORTHREAD_API CT_CORTHREAD* ct_corthread_running(void);

	/**
	* get the corthread ID of the specified corthread
	* @param corthread {const CT_CORTHREAD*} the specified corthread object
	* @return {unsigned int} return the corthread ID
	*/
	CORTHREAD_API unsigned int ct_corthread_id(const CT_CORTHREAD* corthread);

	/**
	* get the current running corthread's ID
	* @return {unsigned int} the current corthread's ID
	*/
	CORTHREAD_API unsigned int ct_corthread_self(void);

	/**
	* set the error number to the specified corthread object
	* @param corthread {CT_CORTHREAD*} the specified corthread, if NULL the current running
	*  corthread will be used
	* @param errnum {int} the error number
	*/
	CORTHREAD_API void ct_corthread_set_errno(CT_CORTHREAD* corthread, int errnum);

	/**
	* get the error number of assosiated corthread
	* @param corthread {CT_CORTHREAD*} the specified corthread, if NULL the current running
	* @return {int} get the error number of assosiated corthread
	*/
	CORTHREAD_API int ct_corthread_errno(CT_CORTHREAD* corthread);

	/**
	* @deprecated
	* @param corthread {CT_CORTHREAD*}
	* @param yesno {int}
	*/
	CORTHREAD_API void ct_corthread_keep_errno(CT_CORTHREAD* corthread, int yesno);

	/**
	* get the assosiated corthread's status
	* @param corthread {CT_CORTHREAD*} the specified corthread, if NULL the current running
	* @return {int}
	*/
	CORTHREAD_API int ct_corthread_status(const CT_CORTHREAD* corthread);

	/**
	* kill the suspended corthread and notify it to exit
	* @param corthread {const CT_CORTHREAD*} the specified corthread, NOT NULL
	*/
	CORTHREAD_API void ct_corthread_kill(CT_CORTHREAD* corthread);

	/**
	* check if the current corthread has been killed
	* @param corthread {CT_CORTHREAD*} the specified corthread, if NULL the current running
	* @return {int} non zero returned if been killed
	*/
	CORTHREAD_API int ct_corthread_killed(CT_CORTHREAD* corthread);

	/**
	* wakeup the suspended corthread with the assosiated signal number
	* @param corthread {const CT_CORTHREAD*} the specified corthread, NOT NULL
	* @param signum {int} SIGINT, SIGKILL, SIGTERM ... refer to bits/signum.h
	*/
	CORTHREAD_API void ct_corthread_signal(CT_CORTHREAD* corthread, int signum);

	/**
	* get the signal number got from other corthread
	* @param corthread {CT_CORTHREAD*} the specified corthread, if NULL the current running
	* @retur {int} the signal number got
	*/
	CORTHREAD_API int ct_corthread_signum(CT_CORTHREAD* corthread);

	/**
	* suspend the current running corthread
	* @return {int}
	*/
	CORTHREAD_API int ct_corthread_yield(void);

	/**
	* add the suspended corthread into resuming queue
	* @param corthread {CT_CORTHREAD*} the corthread, NOT NULL
	*/
	CORTHREAD_API void ct_corthread_ready(CT_CORTHREAD* corthread);

	/**
	* suspend the current corthread and switch to run the next ready corthread
	*/
	CORTHREAD_API void ct_corthread_switch(void);

	/**
	* set the corthread schedule process with automatically, in this way, when one
	* corthread was created, the schedule process will start automatically, but only
	* the first corthread was started, so you can create the other corthreads in this
	* corthread. The default schedule mode is non-automatically, you should call the
	* ct_corthread_schedule or ct_corthread_schedule_with explicit
	*/
	CORTHREAD_API void ct_corthread_schedule_init(int on);

	/**
	* start the corthread schedule process, the corthreads in the ready quque will be
	* started in sequence.
	*/
	CORTHREAD_API void ct_corthread_schedule(void);

	/**
	* start the corthread schedule process with the specified event type, the default
	* event type is CORTHREAD_EVENT_KERNEL. ct_corthread_schedule using the default
	* event type. CORTHREAD_EVENT_KERNEL is diffrent for diffrent OS platform:
	* Linux: epoll; BSD: kqueue; Windows: iocp.
	* @param event_mode {int} the event type, defined as CORTHREAD_EVENT_XXX
	*/

#define CORTHREAD_EVENT_SELECT	2	/* select		*/

	CORTHREAD_API void ct_corthread_schedule_with(int event_mode);

	/**
	* set the event type, the default type is CORTHREAD_EVENT_KERNEL, this function
	* must be called before ct_corthread_schedule.
	* @param event_mode {int} the event type, defined as CORTHREAD_EVENT_XXX
	*/
	CORTHREAD_API void ct_corthread_schedule_set_event(int event_mode);

	/**
	* check if the current thread is in corthread schedule status
	* @return {int} non zero returned if in corthread schedule status
	*/
	CORTHREAD_API int ct_corthread_scheduled(void);

	/**
	* stop the corthread schedule process, all corthreads will be stopped
	*/
	CORTHREAD_API void ct_corthread_schedule_stop(void);

	/**
	* let the current corthread sleep for a while
	* @param milliseconds {unsigned int} the milliseconds to sleep
	* @return {unsigned int} the rest milliseconds returned after wakeup
	*/
	CORTHREAD_API unsigned int ct_corthread_delay(unsigned int milliseconds);

	/**
	* let the current corthread sleep for a while
	* @param seconds {unsigned int} the seconds to sleep
	* @return {unsigned int} the rest seconds returned after wakeup
	*/
	CORTHREAD_API unsigned int ct_corthread_sleep(unsigned int seconds);

	/**
	* create one corthread timer
	* @param milliseconds {unsigned int} the timer wakeup milliseconds
	* @param size {size_t} the virtual memory of the created corthread
	* @param fn {void (*)(CT_CORTHREAD*, void*)} the callback when corthread wakeup
	* @param ctx {void*} the second parameter of the callback fn
	* @return {CT_CORTHREAD*} the new created corthread returned
	*/
	CORTHREAD_API CT_CORTHREAD* ct_corthread_create_timer(unsigned int milliseconds,
		size_t size, void(*fn)(CT_CORTHREAD*, void*), void* ctx);

	/**
	* reset the timer milliseconds time before the timer corthread wakeup
	* @param timer {CT_CORTHREAD*} the corthread created by ct_corthread_create_timer
	* @param milliseconds {unsigned int} the new timer wakeup milliseconds
	*/
	CORTHREAD_API void ct_corthread_reset_timer(CT_CORTHREAD* timer, unsigned int milliseconds);

	/**
	* set the DNS service addr
	* @param ip {const char*} ip of the DNS service
	* @param port {int} port of the DNS service
	*/
	CORTHREAD_API void ct_corthread_set_dns(const char* ip, int port);

	/* for corthread specific */

	/**
	* set the current corthread's local object
	* @param key {int*} the addr of indexed key, its initial value should <= 0,
	*  and one integer which > 0 will be set for it; the corthread local object will
	*  be assosiated with the indexed key.
	* @param ctx {void *} the corthread local object
	* @param free_fn {void (*)(void*)} the callback will be called before the
	*  current corthread exiting
	* @return {int} the integer value(>0) of indexed key returned, value less than
	*  0 will be returned if no running corthread
	*/
	CORTHREAD_API int ct_corthread_set_specific(int* key, void* ctx, void(*free_fn)(void*));

	/**
	* get the current corthread's local object assosiated with the specified indexed key
	* @param key {int} the integer value returned by ct_corthread_set_specific
	* @retur {void*} NULL returned if no corthread local object with the specified key
	*/
	CORTHREAD_API void* ct_corthread_get_specific(int key);

	/****************************************************************************/

	/**
	* log function type used in corthread logging process, should be set by the
	* function ct_corthread_msg_pre_write
	* @param ctx {void*} the user's context
	* @param fmt {const char*} format of parameters
	* @param ap {va_list} list of parameters
	*/
	typedef void(*CORTHREAD_MSG_PRE_WRITE_FN)(void *ctx, const char *fmt, va_list ap);

	/**
	* log function type used in corthread logging process, should be set by the
	* function ct_corthread_msg_register. This can be used by user for get the
	* logging information of corthread
	* @param ctx {void*} the user's context
	* @param fmt {const char*} format of parameters
	* @param ap {va_list} list of parameters
	*/
	typedef void(*CORTHREAD_MSG_WRITE_FN) (void *ctx, const char *fmt, va_list ap);

	/**
	* set the user's log saving function when process started
	* @param write_fn {MSG_WRITE_FN} log function defined by the user
	* @param ctx {void*} parameter will be transfered to write_fn
	*/
	CORTHREAD_API void ct_corthread_msg_register(CORTHREAD_MSG_WRITE_FN write_fn, void *ctx);

	/**
	* cleanup the registered log callback by ct_corthread_msg_register
	*/
	CORTHREAD_API void ct_corthread_msg_unregister(void);

	/**
	* register the user's callback
	* @param pre_write {MSG_PRE_WRITE_FN}
	* @param ctx {void*}
	*/
	CORTHREAD_API void ct_corthread_msg_pre_write(CORTHREAD_MSG_PRE_WRITE_FN pre_write, void *ctx);

	/**
	* if showing the corthread schedule process's log to stdout
	* @param onoff {int} log will be showed to stdout if onoff isn't 0
	*/
	CORTHREAD_API void ct_corthread_msg_stdout_enable(int onoff);

	/**
	* get the system error number of last system API calling
	* @return {int} error number
	*/
	CORTHREAD_API int ct_corthread_last_error(void);

	/**
	* get the error information of last system API calling
	* @return {const char*}
	*/
	CORTHREAD_API const char *ct_corthread_last_serror(void);

	/**
	* set the system error number
	* @param errnum {int} ´íÎóºÅ
	*/
	void ct_corthread_set_error(int errnum);

	/****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif