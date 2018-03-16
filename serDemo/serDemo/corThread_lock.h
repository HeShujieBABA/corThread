#ifndef CORTHREAD_LOCK_INCLUDE_H
#define CORTHREAD_LOCK_INCLUDE_H

#include "corthread_define.h"

#ifdef __cplusplus
extern "C" {
#endif

	/* corthread locking */

	/**
	* corthread mutex, thread unsafety, one corthread mutex can only be used in the
	* same thread, otherwise the result is unpredictable
	*/
	typedef struct CT_CORTHREAD_MUTEX CT_CORTHREAD_MUTEX;

	/**
	* corthread read/write mutex, thread unsafety, can only be used in the sampe thread
	*/
	typedef struct CT_CORTHREAD_RWLOCK CT_CORTHREAD_RWLOCK;

	/**
	* create one corthread mutex, can only be used in the same thread
	* @return {CT_CORTHREAD_MUTEX*} corthread mutex returned
	*/
	CORTHREAD_API CT_CORTHREAD_MUTEX* ct_corthread_mutex_create(void);

	/**
	* free corthread mutex created by ct_corthread_mutex_create
	* @param l {CT_CORTHREAD_MUTEX*} created by ct_corthread_mutex_create
	*/
	CORTHREAD_API void ct_corthread_mutex_free(CT_CORTHREAD_MUTEX* l);

	/**
	* lock the specified corthread mutex, return immediately when locked, or will
	* wait until the mutex can be used
	* @param l {CT_CORTHREAD_MUTEX*} created by ct_corthread_mutex_create
	*/
	CORTHREAD_API void ct_corthread_mutex_lock(CT_CORTHREAD_MUTEX* l);

	/**
	* try lock the specified corthread mutex, return immediately no matter the mutex
	* can be locked.
	* @param l {CT_CORTHREAD_MUTEX*} created by ct_corthread_mutex_create
	* @return {int} 0 returned when locking successfully, -1 when locking failed
	*/
	CORTHREAD_API int ct_corthread_mutex_trylock(CT_CORTHREAD_MUTEX* l);

	/**
	* the corthread mutex be unlock by its owner corthread, fatal will happen when others
	* release the corthread mutex
	* @param l {CT_CORTHREAD_MUTEX*} created by ct_corthread_mutex_create
	*/
	CORTHREAD_API void ct_corthread_mutex_unlock(CT_CORTHREAD_MUTEX* l);

	/****************************************************************************/

	/**
	* create one corthread rwlock, can only be operated in the sampe thread
	* @return {CT_CORTHREAD_RWLOCK*}
	*/
	CORTHREAD_API CT_CORTHREAD_RWLOCK* ct_corthread_rwlock_create(void);

	/**
	* free rw mutex created by ct_corthread_rwlock_create
	* @param l {CT_CORTHREAD_RWLOCK*} created by ct_corthread_rwlock_create
	*/
	CORTHREAD_API void ct_corthread_rwlock_free(CT_CORTHREAD_RWLOCK* l);

	/**
	* lock the rwlock, if there is no any write locking on it, the
	* function will return immediately; otherwise, the caller will wait for all
	* write locking be released. Read lock on it will successful when returning
	* @param l {CT_CORTHREAD_RWLOCK*} created by ct_corthread_rwlock_create
	*/
	CORTHREAD_API void ct_corthread_rwlock_rlock(CT_CORTHREAD_RWLOCK* l);

	/**
	* try to locking the Readonly lock, return immediately no matter locking
	* is successful.
	* @param l {CT_CORTHREAD_RWLOCK*} crated by ct_corthread_rwlock_create
	* @retur {int} 1 returned when successfully locked, or 0 returned if locking
	*  operation is failed.
	*/
	CORTHREAD_API int ct_corthread_rwlock_tryrlock(CT_CORTHREAD_RWLOCK* l);

	/**
	* lock the rwlock in Write Lock mode, return until no any one locking it
	* @param l {CT_CORTHREAD_RWLOCK*} created by ct_corthread_rwlock_create
	*/
	CORTHREAD_API void ct_corthread_rwlock_wlock(CT_CORTHREAD_RWLOCK* l);

	/**
	* try to lock the rwlock in Write Lock mode. return immediately no matter
	* locking is successful.
	* @param l {CT_CORTHREAD_RWLOCK*} created by ct_corthread_rwlock_create
	* @return {int} 1 returned when locking successfully, or 0 returned when
	*  locking failed
	*/
	CORTHREAD_API int ct_corthread_rwlock_trywlock(CT_CORTHREAD_RWLOCK* l);

	/**
	* the rwlock's Read-Lock owner unlock the rwlock
	* @param l {CT_CORTHREAD_RWLOCK*} crated by ct_corthread_rwlock_create
	*/
	CORTHREAD_API void ct_corthread_rwlock_runlock(CT_CORTHREAD_RWLOCK* l);

	/**
	* the rwlock's Write-Lock owner unlock the rwlock
	* @param l {CT_CORTHREAD_RWLOCK*} created by ct_corthread_rwlock_create
	*/
	CORTHREAD_API void ct_corthread_rwlock_wunlock(CT_CORTHREAD_RWLOCK* l);

#ifdef __cplusplus
}
#endif

#endif