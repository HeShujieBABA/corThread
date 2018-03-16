#ifndef SANE_SOCKET_INCLUDE_H
#define SANE_SOCKET_INCLUDE_H

#include "libcorthread.h"

int is_listen_socket(socket_t fd);
int getsocktype(socket_t fd);

#endif
