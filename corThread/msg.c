#define _CRT_SECURE_NO_WARNINGS  
#include <stdio.h> 

#include "stdafx.h"
#include "corThread_base.h"
#include "init.h"
#include "pthread_patch.h"
#include "msg.h"
#include "libcorThread.h"

#ifndef	USE_PRINTF_MACRO

static int __stdout_enable = 0;

static CORTHREAD_MSG_WRITE_FN     __write_fn = NULL;
static CORTHREAD_MSG_PRE_WRITE_FN __pre_write_fn = NULL;

static void *__pre_write_ctx = NULL;
static void *__msg_ctx = NULL;

void ct_corthread_msg_register(CORTHREAD_MSG_WRITE_FN write_fn, void *ctx)
{
	if (write_fn != NULL) {
		__write_fn = write_fn;
		__msg_ctx = ctx;
	}
}

void ct_corthread_msg_unregister(void)
{
	__write_fn = NULL;
	__msg_ctx = NULL;
	__pre_write_fn = NULL;
	__pre_write_ctx = NULL;
}

void ct_corthread_msg_pre_write(CORTHREAD_MSG_PRE_WRITE_FN pre_write, void *ctx)
{
	__pre_write_fn = pre_write;
	__pre_write_ctx = ctx;
}

void ct_corthread_msg_stdout_enable(int onoff)
{
	__stdout_enable = onoff;
}

void msg_info(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	if (__pre_write_fn) {
		__pre_write_fn(__pre_write_ctx, fmt, ap);
	}

	if (__write_fn != NULL) {
		__write_fn(__msg_ctx, fmt, ap);
	}

	if (__stdout_enable) {
		printf("msg_info->pid(%d), ", GETPID());
		vprintf(fmt, ap);
		printf("\r\n");
	}

	va_end(ap);
}

void msg_warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	if (__pre_write_fn) {
		__pre_write_fn(__pre_write_ctx, fmt, ap);
	}

	if (__write_fn != NULL) {
		__write_fn(__msg_ctx, fmt, ap);
	}

	if (__stdout_enable) {
		printf("msg_warn->pid(%d), ", GETPID());
		vprintf(fmt, ap);
		printf("\r\n");
	}

	va_end(ap);
}

void msg_error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	if (__pre_write_fn) {
		__pre_write_fn(__pre_write_ctx, fmt, ap);
	}

	if (__write_fn != NULL) {
		__write_fn(__msg_ctx, fmt, ap);
	}

	if (__stdout_enable) {
		printf("msg_error->pid(%d), ", GETPID());
		vprintf(fmt, ap);
		printf("\r\n");
	}

	va_end(ap);
}

void msg_fatal(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	if (__pre_write_fn) {
		__pre_write_fn(__pre_write_ctx, fmt, ap);
	}

	if (__write_fn != NULL) {
		__write_fn(__msg_ctx, fmt, ap);
	}

	if (__stdout_enable) {
		printf("msg_fatal->pid(%d), ", GETPID());
		printf("fatal:");
		vprintf(fmt, ap);
		printf("\r\n");
	}

	va_end(ap);
	assert(0);
}

#endif  

const char *msg_strerror(int errnum, char *buffer, size_t size)
{
#ifdef SYS_WIN
	int   L;

	if (buffer == NULL || size <= 0) {
		msg_error("%s, %s(%d): input error",
			__FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}

	L = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_ARGUMENT_ARRAY,
		NULL,
		errnum,
		0,
		buffer,
		size,
		NULL);
	while ((L > 0) && ((buffer[L - 1] >= 0 && buffer[L - 1] <= 32) ||
		(buffer[L - 1] == '.')))
	{
		buffer[L - 1] = '\0';
		L--;
	}
#else
	if (buffer == NULL || size <= 0) {
		msg_error("%s, %s(%d): input error",
			__FILE__, __FUNCTION__, __LINE__);
		return NULL;
	}
	snprintf(buffer, size, "%s", strerror(errnum));
#endif

	return buffer;
}

const char *last_strerror(char *buffer, size_t size)
{
	return msg_strerror(ct_corthread_last_error(), buffer, size);
}

static pthread_key_t __errbuf_key;
static char *__main_buf = NULL;

static void thread_free_buf(void *buf)
{
	if (__pthread_self() != main_thread_self()) {
		free(buf);
	}
}

static void main_free_buf(void)
{
	if (__main_buf) {
		free(__main_buf);
	}
}

static void thread_buf_init(void)
{
	if (pthread_key_create(&__errbuf_key, thread_free_buf) != 0) {
		abort();
	}
}

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

const char *last_serror(void)
{
	char *buf;
	int   error = ct_corthread_last_error();
	static size_t __buf_size = 4096;

	if (pthread_once(&once_control, thread_buf_init) != 0) {
		abort();
	}

	buf = pthread_getspecific(__errbuf_key);
	if (buf == NULL) {
		buf = malloc(__buf_size);
		assert(pthread_setspecific(__errbuf_key, buf) == 0);
		if (__pthread_self() == main_thread_self()) {
			__main_buf = buf;
			atexit(main_free_buf);
		}
	}
	return msg_strerror(error, buf, __buf_size);
}

const char *ct_corthread_last_serror(void)
{
	return last_serror();
}

void msg_printf(const char *fmt, ...)
{
	char  buf[2048];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	printf("%s\r\n", buf);
	va_end(ap);
}