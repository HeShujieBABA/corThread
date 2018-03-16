#ifndef MSG_H
#define MSG_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdarg.h>
#include <stdio.h>
#include "define_win.h"

	void PRINTF(1, 2) msg_info(const char *format,...);

	void PRINTF(1, 2) msg_warn(const char *format,...);

	void PRINTF(1, 2) msg_error(const char *format,...);

	void PRINTF(1, 2) msg_fatal(const char *format,...);

	const char *msg_strerror(int errnum, char *buffer, size_t size);

	const char *last_strerror(char *buffer, size_t size);

	const char *last_serror(void);

	void PRINTF(1, 2) msg_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif