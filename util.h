#ifndef UTIL_H
#define UTIL_H
ssize_t send_all(int socket, const void *buffer, size_t length);
int make_socket_non_blocking(int sfd);
void __attribute((cold)) __attribute__((noreturn))
die(const char *msg); /* unlikely to be called */
#endif                /* UTIL_H */