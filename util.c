#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "util.h"

__attribute((cold)) void __attribute__((noreturn)) die(const char *msg) {
#ifdef DEBUG
    perror(msg);
#endif
    exit(EXIT_FAILURE); /* splint warnings about possibly null values are false
                           positives, for some reason it doesn't consider this
                           line when seeing the die() function */
}

ssize_t send_all(int socket, const void *buffer, size_t length) {
    ssize_t offset = 0;
    ssize_t sent_bytes = 0;
    while ((sent_bytes = send(socket, buffer + offset, length - offset,
                              MSG_NOSIGNAL)) > 0) { /* ignore sigpipes */
        offset += sent_bytes;
    }

    return sent_bytes == -1 ? -1 : offset;
}

int make_socket_non_blocking(int sfd) { /* from BASHLITE */
    int flags, s;
    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
#ifdef DEBUG
        perror("fcntl");
#endif
        return -1;
    }
    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1) {
#ifdef DEBUG
        perror("fcntl");
#endif
        return -1;
    }
    return 0;
}
