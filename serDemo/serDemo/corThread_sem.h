#ifndef CORTHREAD_SEM_INCLUDE_H
#define CORTHREAD_SEM_INCLUDE_H

#include "corthread_define.h"

#ifdef __cplusplus
extern "C" {
#endif

	/* corthread semaphore, thread unsafety, one semaphore can only be used in one
	* thread, if used in different threads, result is unpredictable */

	typedef struct CT_CORTHREAD_SEM CT_CORTHREAD_SEM;

	/**
	* create one corthread semaphore, and binding it with the current thread
	* @param num {int} the initial value of the semaphore, must >= 0
	* @return {CT_CORTHREAD_SEM *}
	*/
	CORTHREAD_API CT_CORTHREAD_SEM* ct_corthread_sem_create(int num);

	/**
	* free corthread semaphore
	* @param {CT_CORTHREAD_SEM *}
	*/
	CORTHREAD_API void ct_corthread_sem_free(CT_CORTHREAD_SEM* sem);

	/**
	*  get the thread binding the specificed corthread sem
	* @param sem {CT_CORTHREAD_SEM*} created by ct_corthread_sem_create
	* @return {unsigned long} thread ID of the thread binding the semaphore
	*/
#if !defined(_WIN32) && !defined(_WIN64)
	CORTHREAD_API unsigned long ct_corthread_sem_get_tid(CT_CORTHREAD_SEM* sem);
#endif

	/**
	* set the thread ID the semaphore belongs to, changing the owner of the corthread
	* semaphore, when this function was called, the value of the semaphore must
	* be zero, otherwise fatal will happen.
	* @param sem {CT_CORTHREAD_SEM*} created by ct_corthread_sem_create
	* @param {unsigned long} the thread ID to be specificed with the semaphore
	*/
	CORTHREAD_API void ct_corthread_sem_set_tid(CT_CORTHREAD_SEM* sem, unsigned long tid);

	/**
	* wait for semaphore until > 0, semaphore will be -1 when returned
	* @param sem {CT_CORTHREAD_SEM *} created by ct_corthread_sem_create
	* @return {int} the semaphore value returned, if the caller's thread isn't
	*  same as the semaphore owner's thread, -1 will be returned
	*/
	CORTHREAD_API int ct_corthread_sem_wait(CT_CORTHREAD_SEM* sem);

	/**
	* try to wait semaphore until > 0, if semaphore is 0, -1 returned immediately,
	* otherwise semaphore will be decreased 1 and the semaphore's value is returned
	* @param sem {CT_CORTHREAD_SEM *} created by ct_corthread_sem_create
	* @return {int} value(>=0) returned when waiting ok, otherwise -1 will be
	*  returned if the caller's thread isn't same as the semaphore thread or the
	*  semaphore's value is 0
	*/
	CORTHREAD_API int ct_corthread_sem_trywait(CT_CORTHREAD_SEM* sem);

	/**
	* add 1 to the semaphore, if there are other corthreads waiting for semaphore,
	* one waiter will be wakeuped
	* @param sem {CT_CORTHREAD_SEM *} created by ct_corthread_sem_create
	* @return {int} the current semaphore value returned, -1 returned if the
	*  current thread ID is not same as the semaphore's owner ID
	*/
	CORTHREAD_API int ct_corthread_sem_post(CT_CORTHREAD_SEM* sem);

	/**
	* get the specificed semaphore's value
	* @param sem {CT_CORTHREAD_SEM*} created by ct_corthread_sem_create
	* @retur {int} current semaphore's value returned
	*/
	CORTHREAD_API int ct_corthread_sem_num(CT_CORTHREAD_SEM* sem);

#ifdef __cplusplus
}
#endif

#endif
