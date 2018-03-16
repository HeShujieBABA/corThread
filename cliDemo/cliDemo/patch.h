#ifndef __WIN_PATCH_INCLUDE_H__
#define __WIN_PATCH_INCLUDE_H__

#ifdef __cplusplus
extern "C" {
#endif

# include <winsock2.h>
# define HAVE_NO_GETOPT
# define snprintf _snprintf
extern int   optind;
extern char *optarg;
int getopt(int argc, char * const argv[], char *opts);

void socket_init(void);
void socket_end(void);
void socket_close(SOCKET fd);
SOCKET socket_listen(const char *ip, int port);
SOCKET socket_accept(SOCKET fd);
SOCKET socket_connect(const char *ip, int port);

#ifdef __cplusplus
}
#endif

#endif
