#ifndef CORTHREAD_CHANNEL_INCLUDE_H
#define CORTHREAD_CHANNEL_INCLUDE_H

#include "corthread_define.h"

#ifdef __cplusplus
extern "C" {
#endif

	/* channel communication */

	/**
	* the corthread channel type definition
	*/
	typedef struct CT_CHANNEL CT_CHANNEL;

	/**
	* create one corthread channel
	* @param elemsize {int} the fixed object size transfered in corthread channel
	* @param bufsize {int} the buffered of objects in corthread channel
	* @return {CT_CHANNNEL*}
	*/
	CORTHREAD_API CT_CHANNEL* ct_channel_create(int elemsize, int bufsize);

	/**
	* free corthread channel created by ct_channel_create
	* @param c {CT_CHANNEL*} created by ct_channel_create
	*/
	CORTHREAD_API void ct_channel_free(CT_CHANNEL* c);

	/**
	* send object to specified corthread channel in block mode
	* @param c {CT_CHANNEL*} created by ct_channel_create
	* @param v {void*} the object to be transfered
	* @return {int} value (>= 0) returned
	*/
	CORTHREAD_API int ct_channel_send(CT_CHANNEL* c, void* v);

	/**
	* send object to specified corthread channel in non-block mode, one new object
	* copied from which will be created internal
	* @param c {CT_CHANNEL*} created by ct_channel_create
	* @param v {void*} the object to be transfered
	* @return {int} value (>= 0) returned
	*/
	CORTHREAD_API int ct_channel_send_nb(CT_CHANNEL* c, void* v);

	/**
	* read one object from specified channel in block mode
	* @param c {CT_CHANNEL*} created by ct_channel_create
	* @param v {void*} will store the result
	* @return {int} value(>= 0) returned if get one object
	*/
	CORTHREAD_API int ct_channel_recv(CT_CHANNEL* c, void* v);

	/**
	* read one object from specified channel in non-block ode
	* @param c {CT_CHANNEL*} created by ct_channel_create
	* @param v {void*} will store the result
	* @return {int} value(>= 0) returned if get one object, or NULL returned if
	*  none object been got
	*/
	CORTHREAD_API int ct_channel_recv_nb(CT_CHANNEL* c, void* v);

	/**
	* send object's addr to specified channel in block mode
	* @param c {CT_CHANNEL*} created by ct_channel_create
	* @param v {void*} the addr of the object to be transfered
	* @return {int} value (>= 0) returned
	*/
	CORTHREAD_API int ct_channel_sendp(CT_CHANNEL* c, void* v);

	/**
	* get object's addr from specified channel in block mode
	* @param c {CT_CHANNEL*} created by ct_channel_create
	* @return {void*} non-NULL addr returned
	*/
	CORTHREAD_API void *ct_channel_recvp(CT_CHANNEL* c);

	/**
	* send the object's addr to specified channel in non-block mode
	* @param c {CT_CHANNEL*} created by ct_channel_create
	* @param v {void*} the addr of the object to be transfered
	* @return {int} value which is >= 0 returned
	*/
	CORTHREAD_API int ct_channel_sendp_nb(CT_CHANNEL* c, void* v);

	/**
	* get the object's addr form specified channel in non-block mode
	* @param c {CT_CHANNEL*} created by ct_channel_create
	* @return {void*} * non-NULL returned when got one, or NULL returned
	*/
	CORTHREAD_API void *ct_channel_recvp_nb(CT_CHANNEL* c);

	/**
	* send unsigned integer to specified channel in block mode
	* @param c {CT_CHANNEL*} created by ct_channel_create
	* @param val {unsigned long} the integer to be sent
	* @return {int} value (>= 0) returned
	*/
	CORTHREAD_API int ct_channel_sendul(CT_CHANNEL* c, unsigned long val);

	/**
	* get unsigned integer from specified channel in block mode
	* @param c {CT_CHANNEL*} created by ct_channel_create
	* @return {unsigned long}
	*/
	CORTHREAD_API unsigned long ct_channel_recvul(CT_CHANNEL* c);

	/**
	* sent unsigned integer to specified channel in non-block mode
	* @param c {CT_CHANNEL*} created by ct_channel_create
	* @param val {unsigned long} integer to be sent
	* @return {int} value(>= 0) returned
	*/
	CORTHREAD_API int ct_channel_sendul_nb(CT_CHANNEL* c, unsigned long val);

	/**
	* get one unsigned integer from specified channel in non-block mode
	* @param c {CT_CHANNEL*} created by ct_channel_create
	* @return {unsigned long}
	*/
	CORTHREAD_API unsigned long ct_channel_recvul_nb(CT_CHANNEL* c);

#ifdef __cplusplus
}
#endif

#endif
