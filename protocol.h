#ifndef PROTOCOL_H
#define PROTOCOL_H
int protocol_process_msg(int sockfd, unsigned char *msg);
int protocol_format_send_msg(int sockfd, char *msg, fd_set *master);
int protocol_message_receive_and_process(int server_fd);
#endif /* PROTOCOL_H */