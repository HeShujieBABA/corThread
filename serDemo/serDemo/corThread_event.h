#ifndef CORTHREAD_EVENT_INCLUDE_H
#define CORTHREAD_EVENT_INCLUDE_H

#include "corthread_define.h"

#ifdef __cplusplus
extern "C" {
#endif

	/* corthread_event.c */

	/* corthread event mutex object based on IO event, which is thread safety. That's
	* to say one event object can used in different threads
	*/
	typedef struct CT_CORTHREAD_EVENT CT_CORTHREAD_EVENT;

	/**
	* create corthread event mutex which can be used in corthreads mode or threads mode
	* @return {CT_CORTHREAD_EVENT *}
	*/
	CORTHREAD_API CT_CORTHREAD_EVENT *ct_corthread_event_create(void);

	/**
	* free event mutex returned by ct_corthread_event_create
	* @param {CT_CORTHREAD_EVENT *}
	*/
	CORTHREAD_API void ct_corthread_event_free(CT_CORTHREAD_EVENT *event);

	/**
	* wait for event can be available
	* @param {CT_CORTHREAD_EVENT *}
	* @return {int} 0 returned if successful, or -1 if error happened
	*/
	CORTHREAD_API int ct_corthread_event_wait(CT_CORTHREAD_EVENT *event);

	/**
	* try to wait for event can be available
	* @param {CT_CORTHREAD_EVENT *}
	* @return {int} 0 returned if successful, or -1 if the event been locked
	*/
	CORTHREAD_API int ct_corthread_event_trywait(CT_CORTHREAD_EVENT *event);

	/**
	* the event's owner notify the waiters that the event mutex can be available,
	* and the waiter will get the event mutex
	* @param {CT_CORTHREAD_EVENT *}
	* @return {int} 0 returned if successful, or -1 if error happened
	*/
	CORTHREAD_API int ct_corthread_event_notify(CT_CORTHREAD_EVENT *event);

#ifdef __cplusplus
}
#endif

#endif
