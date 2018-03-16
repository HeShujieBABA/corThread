#ifndef	__DEFINE_INCLUDE_H__
#define	__DEFINE_INCLUDE_H__

#if defined(_WIN32) || defined(_WIN64)
# if(_MSC_VER >= 1300)
#  undef FD_SETSIZE
#  define FD_SETSIZE 10240
#  include <winsock2.h>
#  include <mswsock.h>
# endif
# define SYS_WIN
# define HAS_SELECT

# define __thread __declspec(thread)
typedef unsigned long nfds_t;
#endif

#  define corthread_unused 

#define	PRINTF(format_idx, arg_idx)
#define	SCANF
#define	NORETURN
#define	UNUSED

#define	DEPRECATED __declspec(deprecated)

#define	DEPRECATED_FOR(f) __declspec(deprecated("is deprecated. Use '" #f "' instead"))

#endif 